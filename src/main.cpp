#include <iostream>
#include <assert.h>
#include <protocol.h>
#include <chrono>
#include <server.h>
#include <epoll.h>

using namespace kvstore;

int main()
{   
    try {
        Server server(6381);
        EventLoopState *state = new EpollEventState();
        EventLoop el;
        el.init(state);
        server.init(&el);
        server.start();
    }catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}