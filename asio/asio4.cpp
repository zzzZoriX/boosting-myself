#include <deque>
#include <set>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <array>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <string>

using namespace boost;
namespace ssl = asio::ssl;
using asio::ip::tcp;


// class tsl_session: public std::enable_shared_from_this<tsl_session> {
//     ssl::stream<tcp::socket> stream;
//     std::array<char, 4096> buffer;

//     asio::signal_set sig_set;


//     void close_connection() {
//         stream.async_shutdown([this, self = shared_from_this()](system::error_code ec){
//             if(!ec) {
//                 std::cout << "disconnect" << std::endl;

//                 system::error_code close_ec;
//                 stream.lowest_layer().close(close_ec);
//             }
//         });
//     }

//     void read() {
//         stream.async_read_some(
//             asio::buffer(buffer),
//             [this, self = shared_from_this()](system::error_code ec, std::size_t bytes) {
//                 if(!ec) {
//                     write(bytes);
//                 }
//             }
//         );
//     }

//     void write(std::size_t len) {
//         asio::async_write(
//             stream, 
//             asio::buffer(buffer, len),
//             [this, self = shared_from_this()](system::error_code ec, std::size_t) {
//                 if(!ec) {
//                     read();
//                 }
//             }
//         );
//     }

// public:
//     tsl_session(asio::io_context& ioc, tcp::socket&& sock, ssl::context& ctx):
//         stream(std::move(sock), ctx), sig_set(ioc, SIGINT, SIGTERM) {}

//     void start() {
//         sig_set.async_wait([this, self = shared_from_this()](system::error_code ec, int){
//             if(!ec) {
//                 close_connection();
//             }
//         });

//         stream.async_handshake(
//             ssl::stream_base::server, 
//             [this, self = shared_from_this()](system::error_code ec) {
//                 if(!ec) {
//                     read();                   
//                 }
//                 else {
//                     sig_set.cancel();
//                     close_connection();
//                 }
//             }
//         );
//     }
// };

// class tsl_server {
//     tcp::acceptor acceptor;
//     asio::io_context& ioc;
//     ssl::context& ctx;

//     void do_accept() {
//         acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::make_shared<tsl_session>(ioc, std::move(sock), ctx)->start();

//                 std::cout << "new connection" << std::endl;
//             }
//         }); 
//     }

// public:
//     tsl_server(asio::io_context& ioc, ssl::context& ctx, const int port):
//         acceptor(ioc, tcp::endpoint(tcp::v4(), port)), ctx(ctx), ioc(ioc) 
//     {
//         do_accept();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         ssl::context ctx{ssl::context::tlsv12_server};

//         ctx.set_options(
//             ssl::context::default_workarounds |
//             ssl::context::no_sslv2 |
//             ssl::context::no_sslv3 |
//             ssl::context::single_dh_use
//         );

//         ctx.use_certificate_chain_file("server.crt");
//         ctx.use_private_key_file("server.key", ssl::context::pem);

//         tsl_server serv{ioc, ctx, 1234};
        
//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }


// задания
// 4.1: SSL Echo-Сервер -- выполнено
class tsl_session: public std::enable_shared_from_this<tsl_session> {
    asio::streambuf buffer;
    ssl::stream<tcp::socket> stream;

    asio::strand<asio::any_io_executor> strand;


    void shutdown() {
        stream.async_shutdown(asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec){
            if(!ec) {
                std::cout << "Client " << stream.lowest_layer().remote_endpoint() << " disconnected" << std::endl;
                
                system::error_code close_ec;
                stream.lowest_layer().close(close_ec);
            }
        }));
    }

    void read() {
        asio::async_read_until(
            stream,
            buffer,
            "\n",
            asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
                if(!ec) {
                    std::cout << "new message received" << std::endl;

                    write(len);
                }
                else {
                    std::cout << ec.message() << std::endl;

                    shutdown();
                }
            })
        );
    }

    void write(std::size_t bytes) {
        asio::async_write(
            stream,
            buffer,
            asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t){
                if(!ec) {
                    read();
                }
                else {
                    std::cout << ec.message() << std::endl;

                    shutdown();
                }
            })
        );
    }

public:
    tsl_session(tcp::socket&& sock, ssl::context& ctx, asio::io_context& ioc):
        stream(std::move(sock), ctx), strand(asio::make_strand(ioc)) {}
        
    void start() {
        stream.async_handshake(
            ssl::stream_base::server,
            [this, self = shared_from_this()](system::error_code ec){
                if(!ec) {
                    read();
                }
                else {
                    shutdown();
                }
            }
        );
    }
};

class tsl_server {
    asio::io_context& ioc;
    tcp::acceptor acceptor;
    ssl::context& ctx;

    const int port;


    void do_accept() {
        acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
            if(!ec) {
                std::cout << "new connection from " << sock.remote_endpoint() << std::endl;

                std::make_shared<tsl_session>(std::move(sock), ctx, ioc)->start();
            }
            else {
                std::cout << ec.message() << std::endl;
            }

            do_accept();
        });
    }

public:
    tsl_server(asio::io_context& ioc, const int port, ssl::context& ctx):
        port(port), ioc(ioc), ctx(ctx), acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {}

    void start() {
        std::cout << "server running on 127.0.0.1:" << port << std::endl;

        do_accept();
    }
};


int main() {
    try {
        asio::io_context ioc;

        ssl::context ctx{ssl::context::tlsv12_server};

        ctx.set_options(
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 |
            ssl::context::no_sslv3 |
            ssl::context::single_dh_use
        );

        ctx.use_certificate_chain_file("server.crt");
        ctx.use_private_key_file("server.key", ssl::context::pem);

        tsl_server serv{ioc, 1234, ctx};
        serv.start();

        ioc.run();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;

        return 1;
    }

    return 0;
}