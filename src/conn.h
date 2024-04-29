#ifndef CONN_H
#define CONN_H
#include <string>
#include <netinet/in.h>
namespace kvstore
{
    class ClientConn
    {
    public:
        ClientConn(){}
        ClientConn(int fd): fd(fd) 
        {
            writeBuffer = std::string();
        }
        ~ClientConn()
        {
            
        }
        int getFd()
        {
            return fd;
        }
        int selectedDB;
    private:
        int fd;
        std::string writeBuffer;
        sockaddr_in addr;
    friend class Server;
    };
}
#endif