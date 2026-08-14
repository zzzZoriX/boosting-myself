#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost;
using asio::ip::tcp;


int main() {
    try {
        asio::io_context ioc;

        tcp::resolver resolver(ioc);
        auto ep = resolver.resolve("echo.websocket.event", "80");

        tcp::socket sock(ioc);
        asio::connect(sock, ep);

        std::string req{"GET / HTTP/1.1\r\nHost: echo.websocket.events\r\n\r\n"};
        asio::write(sock, asio::buffer(req));

        std::array<char, 1024> buf;
        boost::system::error_code ec;

        auto len = sock.read_some(asio::buffer(buf), ec);

        sock.close();

        if(ec == asio::error::eof) {
            std::cout << "wth " << ec.message() << std::endl;
            
            return 1;
        }

        std::cout.write(buf.data(), len);

    } 
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}