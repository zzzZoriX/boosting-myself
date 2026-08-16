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


class UDP_server {
    udp::endpoint sender_ep;
    std::array<char, 65535> buffer;
    udp::socket sock;

    
    void receive() {
        sock.async_receive_from(
            asio::buffer(buffer),
            sender_ep,
            [this](system::error_code ec, std::size_t recv_len) {
                if(!ec && recv_len > 0) {
                    std::cout << "message received from " << sender_ep << std::endl;

                    send(recv_len);
                }
                else {
                    receive();
                }
            }
        );
    }

    void send(std::size_t recv_len) {
        sock.async_send_to(
            asio::buffer(buffer, recv_len),
            sender_ep,
            [this](system::error_code, std::size_t) {
                receive();
            }
        );
    }
    
public:
    UDP_server(asio::io_context& ioc, const int port):
        sock(ioc, udp::endpoint(udp::v4(), port)) 
    {
        receive();
    }
};


int main() {
    try {
        asio::io_context ioc;

        UDP_server serv(ioc, 1234);

        ioc.run();
    }
    catch(const std::exception& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}