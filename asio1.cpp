#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost;
using asio::ip::tcp;


// int main() {
//     try {
//         asio::io_context ioc;

//         tcp::resolver resolver(ioc);
//         auto ep = resolver.resolve("echo.websocket.event", "80");

//         tcp::socket sock(ioc);
//         asio::connect(sock, ep);

//         std::string req{"GET / HTTP/1.1\r\nHost: echo.websocket.events\r\n\r\n"};
//         asio::write(sock, asio::buffer(req));

//         std::array<char, 1024> buf;
//         boost::system::error_code ec;

//         auto len = sock.read_some(asio::buffer(buf), ec);

//         sock.close();

//         if(ec == asio::error::eof) {
//             std::cout << "wth " << ec.message() << std::endl;
            
//             return 1;
//         }

//         std::cout.write(buf.data(), len);

//     } 
//     catch (const std::exception& e) {
//         std::cout << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }

// practice
int main() {
    try {
        asio::io_context ioc;

        std::string host{"api.open-meteo.com"},
                    port{"80"},
                    target{"/v1/forecast?latitude=55.7512&longitude=37.6184&current_weather=true"};

        tcp::resolver resolver{ioc};
        auto eps{resolver.resolve(host, port)};

        tcp::socket sock{ioc};
        asio::connect(sock, eps);

        std::string request = // сгенерировал запрос с помощью ИИ(пока не разбираюсь в том, как их писать самому)
            "GET " + target + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: PureAsioClient/1.0\r\n" +
            "Accept: */*\r\n" +
            "Connection: close\r\n\r\n";

        asio::write(sock, asio::buffer(request));
    
        std::array<char, 4096> buffer; // не юзайте больше никогда streambuf)))0
        system::error_code ec;

        auto len = sock.read_some(asio::buffer(buffer));
        sock.close();

        std::cout << buffer.data() << std::endl;

    }
    catch(const std::exception& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}