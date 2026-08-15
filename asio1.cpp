#include <chrono>
#include <memory>
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
// int main() {
//     try {
//         asio::io_context ioc;

//         std::string host{"api.open-meteo.com"},
//                     port{"80"},
//                     target{"/v1/forecast?latitude=55.7512&longitude=37.6184&current_weather=true"};

//         tcp::resolver resolver{ioc};
//         auto eps{resolver.resolve(host, port)};

//         tcp::socket sock{ioc};
//         asio::connect(sock, eps);

//         std::string request = // сгенерировал запрос с помощью ИИ(пока не разбираюсь в том, как их писать самому)
//             "GET " + target + " HTTP/1.1\r\n" +
//             "Host: " + host + "\r\n" +
//             "User-Agent: PureAsioClient/1.0\r\n" +
//             "Accept: */*\r\n" +
//             "Connection: close\r\n\r\n";

//         asio::write(sock, asio::buffer(request));
    
//         std::array<char, 4096> buffer;
//         system::error_code ec;

//         auto len = sock.read_some(asio::buffer(buffer));
//         sock.close();

//         std::cout << buffer.data() << std::endl;

//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

/*
    я больше не смог придумать практических заданий, которые будут занимать ну минут 10 максимум, чисто руки набить да запомнить че да как, поэтому сразу перейду к асинку
*/

// async
// class session: public std::enable_shared_from_this<session> {
//     asio::streambuf buffer;
//     tcp::socket sock;


//     void read() {
//         asio::async_read_until(
//             sock,
//             buffer,
//             "\n",
//             [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
//                 if(!ec) {
//                     std::cout << "message received" << std::endl;

//                     write(len);
//                 }
//                 else {
//                     std::cout << ec.message() << std::endl;
//                     return;
//                 }
//             }
//         );
//     }

//     void write(const std::size_t len) {
//         asio::async_write(
//             sock,
//             asio::buffer(buffer.data(), len),
//             [this, len, self = shared_from_this()](system::error_code ec, std::size_t) {
//                 if(!ec) {
//                     buffer.consume(len);

//                     read();
//                 }
//             }
//         );
//     }

// public:
//     session(tcp::socket&& sock): sock(std::move(sock)) {}

//     void start() {
//         read();
//     }
// };

// class server {
//     tcp::acceptor acceptor;


//     void do_accept() {
//         acceptor.async_accept(
//             [this](system::error_code ec, tcp::socket sock) {
//                 std::cout << "New connection: " << sock.local_endpoint() << std::endl;

//                 if(!ec) std::make_shared<session>(std::move(sock))->start();

//                 do_accept();
//             }
//         );
//     }

// public:
//     server(asio::io_context& ioc, std::uint16_t port): acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
//         do_accept();       
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         server server(ioc, 1234);

//         std::cout << "Server start listening at port 1234" << std::endl;

//         ioc.run();
//     }
//     catch (const std::exception& e) {
//         std::cout << e.what() << std::endl;
    
//         return 1;
//     }

//     return 0;
// } // работает


// class timer_work: public std::enable_shared_from_this<timer_work> {
//     int times;
//     asio::steady_timer timer;
//     asio::io_context& ioc;


//     void iteration() {
//         std::cout << "Time remain " << times << std::endl;

//         if(times-- <= 0) {
//             ioc.stop();

//             std::cout << "Timer expired" << std::endl;

//             return;
//         }

//         timer.expires_after(std::chrono::seconds(1));

//         timer.async_wait([this, self = shared_from_this()](system::error_code ec){
//             if(!ec) iteration();
//             else {
//                 std::cout << ec.message() << std::endl;

//                 timer.cancel();

//                 return;
//             }
//         });
//     }

// public:
//     timer_work(asio::io_context& ioc):  ioc(ioc),
//                                         timer(ioc) {}

//     void start(const int times) {
//         this->times = times;

//         iteration();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         auto tw = std::make_shared<timer_work>(ioc);
//         tw->start(5);

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what();

//         return 1;
//     }

//     return 0;
// }

/*
    задание 1.3 - выполнено
    задание 1.1 - выполнено (изи)
*/