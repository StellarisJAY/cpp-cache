#include <iostream>
#include <assert.h>
#include <protocol.h>
#include <chrono>
#include <server.h>
#include <epoll.h>

// using namespace std::chrono;
using namespace kvstore;

// void benchmarkCodec()
// {
//     {
//         RedisCodec codec;
//         std::string raw = "#3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$1\r\n1\r\n";
//         uint64_t N = 10000000;
//         auto start = high_resolution_clock::now();
//         for (uint64_t i = 0; i < N; i++) {
//             auto dec = codec.decode(raw);
//             assert(dec.has_value());
//             assert(dec.value().getType() == ARRAY);
//         }
//         auto end = high_resolution_clock::now();
//         auto duration = nanoseconds(end-start).count() / 1000000;
//         std::cout << "decode time used:" << duration << "ms; " << N * 1000/ duration << "op/s" << std::endl;

//         auto command = codec.decode(raw).value();
//         start = high_resolution_clock::now();
//         for (uint64_t i = 0; i < N; i++) {
//             auto enc = codec.encode(command);
//             assert(enc.length() == raw.length());
//         }
//         end = high_resolution_clock::now();
//         duration = nanoseconds(end-start).count() / 1000000;
//         std::cout << "encode time used:" << duration << "ms; " << N * 1000/ duration << "op/s" << std::endl;
//     }
// }

void testDecode()
{
    RedisCodec codec;
    
}

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