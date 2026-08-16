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
// class session: public std::enable_shared_from_this<session> {
//     int timeout;
//     asio::steady_timer timer;
//     tcp::socket sock;
//     asio::streambuf buffer;


//     void read() {
//         timer.expires_after(std::chrono::seconds(timeout));

//         timer.async_wait([this, self = shared_from_this()](system::error_code ec){
//             if(!ec) {
//                 std::cout << "Client on " << sock.local_endpoint() << " disconnected: Timeout" << std::endl;

//                 close_conn();

//                 return;
//             }
//         });

//         asio::async_read_until(
//             sock,
//             buffer,
//             "\n",
//             [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
//                 timer.cancel();

//                 if(!ec) {
//                     write(len);
//                 }
//                 else {
//                     std::cerr << ec.message() << std::endl;

//                     close_conn();
//                 }
//             }
//         );
//     }

//     void write(const std::size_t len) {
//         asio::async_write(
//             sock,
//             buffer,
//             [this, self = shared_from_this()](system::error_code ec, std::size_t){
//                 if(!ec) {
//                     read();
//                 }
//                 else {
//                     std::cout << ec.message() << std::endl;

//                     close_conn();
//                 }
//             }
//         );
//     }

//     void close_conn() {
//         system::error_code close_ec;

//         sock.close(close_ec);

//         if(close_ec && close_ec != asio::error::operation_aborted)
//             std::cout << close_ec.message() << std::endl;
//     }

// public:
//     session(int timeout, tcp::socket&& sock, asio::io_context& ioc):    timeout(timeout),
//                                                                         sock(std::move(sock)),
//                                                                         timer(ioc) {}

//     void start() {
//         read();
//     }
// };

// class server {
//     asio::io_context& ioc;
//     tcp::acceptor acceptor;
//     int timeout;

    
//     void do_accept() {
//         acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::cout << "New connection from " << sock.local_endpoint() << std::endl;

//                 std::make_shared<session>(timeout, std::move(sock), ioc)->start();
                
//                 do_accept();
//             }
//             else {
//                 std::cout << ec.message() << std::endl;
//             }
//         });
//     }
    
// public:
//     server(asio::io_context& ioc, const int port, const int timeout):   acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
//                                                                         ioc(ioc),
//                                                                         timeout(timeout) {}

//     void start() {
//         std::cout << "Server starts listening on your port" << std::endl;

//         do_accept();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         server serv{ioc, 12345, 3};

//         serv.start();

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cerr << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 2.3 -- выполнено. это пизджец какой-то)
// class chatroom;

// class session: public std::enable_shared_from_this<session> {
//     std::deque<std::string> write_queue;

//     tcp::socket sock;
//     std::array<char, 4096> buffer;
//     asio::strand<asio::any_io_executor> strand;

//     std::weak_ptr<chatroom> chat;


//     void close_connection();
//     void do_write();
//     void read();

// public:
//     session(tcp::socket&& sock, std::shared_ptr<chatroom> chat):
//         sock(std::move(sock)), strand(sock.get_executor()), chat(chat)
//         {}

//     ~session() {
//         system::error_code ec;

//         sock.close(ec);
//     }

//     void start() {
//         read();
//     }

//     void send(std::string message) {
//         asio::post(strand, [this, self = shared_from_this(), msg = std::move(message)](){
//             bool wip{!write_queue.empty()};
//             write_queue.push_back(std::move(msg));

//             if(!wip) {
//                 do_write();               
//             }
//         });
//     }

//     tcp::endpoint get_ep() {
//         return sock.local_endpoint();
//     }
// };

// class chatroom {
//     std::set<std::shared_ptr<session>> sessions;
    
// public:
//     void join(std::shared_ptr<session> session){
//         sessions.emplace(session);
//     }

//     void leave(std::shared_ptr<session> session) {
//         sessions.erase(session);
//     }

//     void deliver(const std::string& sender_msg, std::shared_ptr<session> sender) {
//         std::stringstream msgs;

//         msgs << sender->get_ep() << ": " << sender_msg << std::endl;

//         std::string msg{msgs.str()};

//         for(const auto& s: sessions) {
//             if(s != sender) {
//                 s->send(msg);
//             }
//         }
//     }
// };

// void session::close_connection() {
//     system::error_code ec;

//     sock.close(ec);

//     if (ec) std::cout << ec.message() << std::endl;

//     if(auto chat_ptr = chat.lock()) {
//         chat_ptr->leave(shared_from_this());
//     }
// }

// void session::do_write() {
//     asio::async_write(
//         sock,
//         asio::buffer(write_queue.front()),
//         asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t) {
//             if(!ec) {
//                 write_queue.pop_front();

//                 if(!write_queue.empty()){
//                     do_write();
//                 }
//             }
//             else {
//                 std::cout << ec.message() << std::endl;

//                 write_queue.clear();
//             } 
//         })
//     );
// }

// void session::read() {
//     sock.async_read_some(
//         asio::buffer(buffer),
//         [this, self = shared_from_this()](system::error_code ec, std::size_t len){
//             if (!ec){
//                 std::string msg{buffer.data()};

//                 if(auto chat_ptr = chat.lock()) {
//                     chat_ptr->deliver(msg, shared_from_this());
//                 }

//                 read();
//             }
//             else{
//                 std::cout << ec.message() << std::endl;

//                 close_connection();
//             }
//         }
//     );
// }

// class server {
//     std::shared_ptr<chatroom> chat;
    
//     tcp::acceptor acceptor;
//     int port;


//     void do_accept() {
//         acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::cout << "New connection: " << sock.local_endpoint() << std::endl;

//                 auto session_ptr = std::make_shared<session>(std::move(sock), chat);

//                 chat->join(session_ptr);

//                 session_ptr->start();

//                 do_accept();
//             }
//             else{
//                 std::cout << ec.message() << std::endl;
//             }
//         });
//     }

// public:
//     server(asio::io_context& ioc, const int port):  
//         port(port), acceptor(ioc, tcp::endpoint(tcp::v4(), port)), chat(std::make_shared<chatroom>())
//         {}

//     void start() {
//         do_accept();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         server serv{ioc, 1234};

//         serv.start();

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;
    
//         return 1;
//     }

//     return 0;
// }