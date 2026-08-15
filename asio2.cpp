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


// int main() {
//     try {
//         asio::io_context ioc;

//         asio::strand strand = asio::make_strand(ioc);

//         int shared_counter = 0;

//         for (int i = 0; i < 5; ++i) {
//             asio::post(strand, [&shared_counter, i](){
//                 std::cout << "Task " << i << "Task ID " << std::this_thread::get_id() << std::endl;

//                 ++shared_counter;
//             });
//         }

//         std::vector<std::thread> threads;
        
//         for(int i = 0; i < 4; ++i)
//             threads.emplace_back([&ioc](){
//                 ioc.run();
//             });

//         for(auto& t: threads) t.join();
//     }
//     catch (const std::exception& e) {
//         std::cerr << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }


// задания 
// 2.1 -- выполнено
// int main() {
//     try {
//         asio::io_context ioc;

//         auto strand{asio::make_strand(ioc)};

//         std::vector<int> task21;

//         for(int i = 0; i < 100; ++i) {
//             asio::post(strand, [&task21, i](){
//                 std::cout << "Thread " << std::this_thread::get_id() << " increment the vector" << std::endl;

//                 task21.push_back(i);
//             });
//         }

//         std::vector<std::thread> threads;

//         for(int i = 0; i < 3; ++i) {
//             threads.emplace_back([&ioc](){
//                 ioc.run();
//             });
//         }

//         for(auto& t: threads) t.join();

//         std::cout << "Total vector size: " << task21.size() << std::endl;
//     }
//     catch(const std::exception& e) {
//         std::cerr << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 2.2 -- выполнено
class session: public std::enable_shared_from_this<session> {
    int timeout;
    asio::steady_timer timer;
    tcp::socket sock;
    asio::streambuf buffer;


    void read() {
        timer.expires_after(std::chrono::seconds(timeout));

        timer.async_wait([this, self = shared_from_this()](system::error_code ec){
            if(!ec) {
                std::cout << "Client on " << sock.local_endpoint() << " disconnected: Timeout" << std::endl;

                close_conn();

                return;
            }
        });

        asio::async_read_until(
            sock,
            buffer,
            "\n",
            [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
                timer.cancel();

                if(!ec) {
                    write(len);
                }
                else {
                    std::cerr << ec.message() << std::endl;

                    close_conn();
                }
            }
        );
    }

    void write(const std::size_t len) {
        asio::async_write(
            sock,
            buffer,
            [this, self = shared_from_this()](system::error_code ec, std::size_t){
                if(!ec) {
                    read();
                }
                else {
                    std::cout << ec.message() << std::endl;

                    close_conn();
                }
            }
        );
    }

    void close_conn() {
        system::error_code close_ec;

        sock.close(close_ec);

        if(close_ec && close_ec != asio::error::operation_aborted)
            std::cout << close_ec.message() << std::endl;
    }

public:
    session(int timeout, tcp::socket&& sock, asio::io_context& ioc):    timeout(timeout),
                                                                        sock(std::move(sock)),
                                                                        timer(ioc) {}

    void start() {
        read();
    }
};

class server {
    asio::io_context& ioc;
    tcp::acceptor acceptor;
    int timeout;

    
    void do_accept() {
        acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
            if(!ec) {
                std::cout << "New connection from " << sock.local_endpoint() << std::endl;

                std::make_shared<session>(timeout, std::move(sock), ioc)->start();
                
                do_accept();
            }
            else {
                std::cout << ec.message() << std::endl;
            }
        });
    }
    
public:
    server(asio::io_context& ioc, const int port, const int timeout):   acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
                                                                        ioc(ioc),
                                                                        timeout(timeout) {}

    void start() {
        std::cout << "Server starts listening on your port" << std::endl;

        do_accept();
    }
};


int main() {
    try {
        asio::io_context ioc;

        server serv{ioc, 12345, 3};

        serv.start();

        ioc.run();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;

        return 1;
    }

    return 0;
}