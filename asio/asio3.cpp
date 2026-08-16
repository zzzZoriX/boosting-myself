#include <deque>
#include <set>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost;
using asio::ip::tcp;
using asio::ip::udp;


// class UDP_server {
//     udp::endpoint sender_ep;
//     std::array<char, 65535> buffer;
//     udp::socket sock;

    
//     void receive() {
//         sock.async_receive_from(
//             asio::buffer(buffer),
//             sender_ep,
//             [this](system::error_code ec, std::size_t recv_len) {
//                 if(!ec && recv_len > 0) {
//                     std::cout << "message received from " << sender_ep << std::endl;

//                     send(recv_len);
//                 }
//                 else {
//                     receive();
//                 }
//             }
//         );
//     }

//     void send(std::size_t recv_len) {
//         sock.async_send_to(
//             asio::buffer(buffer, recv_len),
//             sender_ep,
//             [this](system::error_code, std::size_t) {
//                 receive();
//             }
//         );
//     }
    
// public:
//     UDP_server(asio::io_context& ioc, const int port):
//         sock(ioc, udp::endpoint(udp::v4(), port)) 
//     {
//         receive();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         UDP_server serv(ioc, 1234);


//         asio::signal_set sigs{ioc, SIGINT, SIGTERM};
//         sigs.async_wait([&ioc](system::error_code ec, int signum){
//             if(!ec) {
//                 std::cout << "Stop signal received. Server stoped.\nSignal number: " << signum << std::endl;

//                 ioc.stop();
//             }
//         });

//         std::cout << "Server running on port 1234. Press ctrl+c to stop." << std::endl;

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// задания
// 3.1 -- выполнено
class udp_heartbeat {
    asio::strand<asio::any_io_executor> strand;

    udp::socket sock;
    udp::endpoint sender_ep;
    std::array<char, 65535> buffer;
    std::string answer = "PONG";


    void receive() {
        sock.async_receive_from(
            asio::buffer(buffer),
            sender_ep,
            asio::bind_executor(strand, [this](system::error_code ec, std::size_t bytes_recv) {
                if(!ec && bytes_recv > 0) {
                    std::cout << "new message received from " << sender_ep << std::endl;

                    send();
                }
                else {
                    receive();
                }
            })
        );
    }

    void send() {
        sock.async_send_to(
            asio::buffer(answer),
            sender_ep,
            asio::bind_executor(strand, [this](system::error_code, std::size_t) {
                receive();
            })
        );
    }

public:
    udp_heartbeat(asio::io_context& ioc, const int port):
        sock(ioc, udp::endpoint(udp::v4(), port)), strand(asio::make_strand(ioc))
    {
        receive();
    }
};


int main() {
    try {
        asio::io_context ioc;

        udp_heartbeat hb{ioc, 54321};

        std::cout << "server running on port 54321" << std::endl;

        ioc.run();
    }
    catch(const std::exception& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}