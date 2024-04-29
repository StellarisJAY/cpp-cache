#include <eventloop.h>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>

namespace kvstore 
{

    void EventLoop::add(int fd, int masks)
    {
        state->add(fd, masks);
    }

    void EventLoop::remove(int fd)
    {
        state->remove(fd);
    }

    void EventLoop::init(EventLoopState *state)
    {
        this->state = state;
    }

    EventLoop::EventLoop(){}

    EventLoop::~EventLoop()
    {
        delete state;
    }

    std::optional<std::vector<ReadyEvent>> EventLoop::poll()
    {
        state->wait(128);
        if (state->readyCount == 0) {
            return std::nullopt;
        }
        return std::make_optional<std::vector<ReadyEvent>>(state->readyQueue);
    }
}