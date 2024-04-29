#ifndef EPOLL_H
#define EPOLL_H
#include <eventloop.h>
#include <vector>
#include <sys/epoll.h>
namespace kvstore
{
    class EpollEventState: public EventLoopState
    {
    public:
        EpollEventState();
        ~EpollEventState();
        void wait(int max);
        void add(int fd, int masks);
        void remove(int fd);
    private:
        int epfd;
        std::vector<epoll_event> events;
    };

}
#endif