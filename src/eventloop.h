#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <optional>
#include <vector>
#define EVENT_NONE 0
#define EVENT_READ 1
#define EVENT_WRITE 2
#define EVENT_HUP 4
#define EVENT_ERR 8


namespace kvstore 
{
    class ReadyEvent
    {
    public:
        int fd;
        int masks;
    };

    class EventLoopState
    {
    public:
        virtual void wait(int max)=0;
        virtual void add(int fd, int masks)=0;
        virtual void remove(int fd)=0;
    protected:
        std::vector<ReadyEvent> readyQueue;
        int readyCount = 0;
    friend class EventLoop;
    };

    class EventLoop
    {
    public:
        void add(int fd, int masks);
        void remove(int fd);
        void init(EventLoopState *state);
        std::optional<std::vector<ReadyEvent>> poll();
        EventLoop();
        ~EventLoop();
    private:
        EventLoopState *state = nullptr;
    };
}
#endif