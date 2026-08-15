#include <memory>
#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>

using namespace boost;
using asio::ip::tcp;


class client: public std::enable_shared_from_this<client> {
    tcp::socket sock;
    std::array<char, 2048> buffer;
    std::string message = "Hello!\n";


    void write() {
        asio::async_write(sock, asio::buffer(message), [this, self = shared_from_this()](system::error_code ec, std::size_t){
            if(!ec) read();
        });
    }

    void read() {
        sock.async_read_some(asio::buffer(buffer), [this, self = shared_from_this()](system::error_code ec, std::size_t){
            if(!ec) {
                std::cout << buffer.data() << std::endl;
                std::cout << "close connection" << std::endl;

                sock.close();
            }
        });
    }

public:
    client(asio::io_context& ioc):  sock(ioc) {}

    void connect(std::uint16_t port) {
        auto ep = tcp::endpoint(tcp::v4(), port);

        sock.async_connect(ep, [this, self = shared_from_this()](system::error_code ec){
            if(!ec) {
                std::cout << "Successfully connected to localhost" << std::endl;

                write();
            }
        });
    }
};

int main() {
    try {
        asio::io_context ioc;

        auto c = std::make_shared<client>(ioc);

        c->connect(1234);

        ioc.run();
    }
    catch(const std::exception& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}

/*
    задание 1.2 - выполнено
*/