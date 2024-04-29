#include <epoll.h>
#include <unistd.h>
#include <iostream>
namespace kvstore
{
    EpollEventState::EpollEventState()
    {
        int epfd = epoll_create(1024);
        this->epfd = epfd;
        this->events = std::vector<epoll_event>(1024);
        this->readyQueue = std::vector<ReadyEvent>();
        this->readyCount = 0;
    }

    EpollEventState::~EpollEventState()
    {
        close(epfd);        
    }

    void EpollEventState::wait(int max)
    {
        int n = epoll_wait(epfd, &events[0], max, -1);
        if (n < 0) {
            readyCount = 0;
            return;
        }
        readyCount = n;
        readyQueue.resize(n);
        for (int i = 0; i < n; i++) {
            ReadyEvent readyEv;
            readyEv.fd = events[i].data.fd;
            int masks = EVENT_NONE;
            if (events[i].events & EPOLLIN) masks |= EVENT_READ;
            if (events[i].events & EPOLLOUT) masks |= EVENT_WRITE;
            if (events[i].events & EPOLLRDHUP) masks |= EVENT_HUP;
            if (events[i].events & EPOLLERR) masks |= EVENT_ERR;
            readyEv.masks = masks;
            readyQueue[i] = readyEv;
        }
    }

    void EpollEventState::add(int fd, int masks)
    {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = 0;
        if (masks & EVENT_READ) ev.events |= EPOLLIN;
        if (masks & EVENT_WRITE) ev.events |= EPOLLOUT;
        if (masks & EVENT_ERR) ev.events |= EPOLLERR;
        if (masks & EVENT_HUP) ev.events |= EPOLLRDHUP;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev)<0) {
            std::cout << "epoll add failed" << errno << " epfd: " << epfd << " conn fd: " << fd << std::endl;
        }
    }

    void EpollEventState::remove(int fd)
    {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = 0;
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    }
}