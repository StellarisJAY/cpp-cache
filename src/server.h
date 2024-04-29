#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <unistd.h>
#include <eventloop.h>
#include <netinet/in.h>
#include <protocol.h>
#include <db.h>
#include <conn.h>
namespace kvstore
{
    class Server
    {
    public:
        Server(uint16_t port);
        ~Server();
        void init(EventLoop *eventloop);
        void acceptConn();
        void closeConn(std::shared_ptr<ClientConn> conn);
        void handleRead(std::shared_ptr<ClientConn> conn);
        void handleWrite(std::shared_ptr<ClientConn> conn);
        void start();
        void closeServer();
    private:
        EventLoop *eventLoop;
        uint16_t port;
        std::unordered_map<int, std::shared_ptr<ClientConn>> conns;
        RedisCodec codec;
        Database db;
        int sockFd;
    };
}

#endif