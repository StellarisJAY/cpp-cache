#include <server.h>
#include <sys/socket.h>
#include <iostream>
#include <exception>

namespace kvstore
{
    Server::Server(uint16_t port): port(port)
    {
        int sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd < 0) {
            throw "create socket error: " + errno;
        }
        sockaddr_in addr;
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        socklen_t len = sizeof(addr);
        if (bind(sockFd, (sockaddr*)(&addr), len) < 0) {
            close(sockFd);
            throw "socket bind error: "+errno;
        }
        this->sockFd = sockFd;
    }

    Server::~Server()
    {

    }

    void Server::init(EventLoop *el)
    {   
        eventLoop = el;
        eventLoop->add(sockFd, EVENT_READ);
    }

    void Server::start()
    {
        if (listen(sockFd, 128) < 0) {
            throw "listen error: " + errno;
        }
        std::cout << "server started, sockFd=" << sockFd << " listening: " << port << std::endl;
        while(1) {
            auto poll = eventLoop->poll();
            if (!poll.has_value()) continue;
            auto events = poll.value();
            for (auto event = events.begin(); event != events.end(); event++) {
                if (event->fd == sockFd) {
                    acceptConn();
                    continue;
                }
                auto conn = conns[event->fd];
                if (conn == nullptr) continue;
                if(event->masks & EVENT_HUP) {
                    closeConn(conn);
                    continue;
                }
                if(event->masks & EVENT_READ) handleRead(conn);
                if(event->masks & EVENT_WRITE) handleWrite(conn);
            }
        }
    }

    void Server::acceptConn()
    {
        std::shared_ptr<ClientConn> conn = std::make_shared<ClientConn>();
        socklen_t addrLen = sizeof(struct sockaddr_in);
        struct sockaddr_in addr;
        int connFd = accept(sockFd, (struct sockaddr*)&addr, &addrLen);
        if (connFd < 0) {
            std::cout << "accept error" << errno << std::endl;
            return;
        }
        eventLoop->add(connFd, EVENT_READ | EVENT_WRITE | EVENT_HUP);
        conn->fd = connFd;
        conn->addr = addr;
        conns[connFd] = conn;
    }

    void Server::closeConn(std::shared_ptr<ClientConn> conn)
    {
        eventLoop->remove(conn->fd);
        conns.erase(conn->fd);
        close(conn->fd);
    }

    void Server::handleRead(std::shared_ptr<ClientConn> conn)
    {
        std::string buf;
        buf.resize(128);
        int n = read(conn->fd, (void *)buf.c_str(), 128);
        if (n < 0) {
            std::cout << "read error: " << errno << std::endl;
            return;
        }
        auto reqBuf = buf.substr(0, n);
        std::optional<RESPCommand> dec = codec.decode(reqBuf);
        if (dec.has_value()) {
            // todo db handle
            RESPCommand resp;
            db.handleCommand(dec.value(), resp, conn);
            conn->writeBuffer = codec.encode(resp);
        }else {
            return;
        }
    }

    void Server::handleWrite(std::shared_ptr<ClientConn> conn)
    {
        if (conn->writeBuffer.length() == 0) {
            return;
        }
        int n = write(conn->fd, (void *)conn->writeBuffer.begin().base(), conn->writeBuffer.length());
        conn->writeBuffer.clear();
    }

    void Server::closeServer()
    {

    }
}