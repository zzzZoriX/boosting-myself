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
// class udp_heartbeat {
//     asio::strand<asio::any_io_executor> strand;

//     udp::socket sock;
//     udp::endpoint sender_ep;
//     std::array<char, 65535> buffer;
//     std::string answer = "PONG";


//     void receive() {
//         sock.async_receive_from(
//             asio::buffer(buffer),
//             sender_ep,
//             asio::bind_executor(strand, [this](system::error_code ec, std::size_t bytes_recv) {
//                 if(!ec && bytes_recv > 0) {
//                     std::cout << "new message received from " << sender_ep << std::endl;

//                     send();
//                 }
//                 else {
//                     receive();
//                 }
//             })
//         );
//     }

//     void send() {
//         sock.async_send_to(
//             asio::buffer(answer),
//             sender_ep,
//             asio::bind_executor(strand, [this](system::error_code, std::size_t) {
//                 receive();
//             })
//         );
//     }

// public:
//     udp_heartbeat(asio::io_context& ioc, const int port):
//         sock(ioc, udp::endpoint(udp::v4(), port)), strand(asio::make_strand(ioc))
//     {
//         receive();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         udp_heartbeat hb{ioc, 54321};

//         std::cout << "server running on port 54321" << std::endl;

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 3.3 -- выполнено
// int main() {
//     try {
//         asio::io_context ioc;

//         asio::strand strand{asio::make_strand(ioc)};
//         asio::strand strand_b{asio::make_strand(ioc)};

//         asio::steady_timer timer{ioc};

//         timer.expires_after(std::chrono::seconds(2));
//         timer.async_wait(asio::bind_executor(strand, [=](system::error_code ec){
//             if(ec) return;

//             std::cout << "strand 1  " << std::this_thread::get_id() << std::endl;

//             asio::post(asio::bind_executor(strand_b, []{
//                 std::cout << "strand 2  " << std::this_thread::get_id() << std::endl;
//             }));
//         }));

//         std::vector<std::thread> threads;
        
//         for(int i = 0; i < 2; ++i) {
//             threads.emplace_back([&ioc]{
//                 ioc.run();
//             });
//         }

//         for(auto& t: threads)
//             t.join();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 3.2 -- выполнено
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
//     bool stopping;

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
//     asio::strand<asio::any_io_executor> strand;
    
// public:
//     chatroom(asio::io_context& ioc):
//         strand(asio::make_strand(ioc))
//         {}

//     void join(std::shared_ptr<session> session){
//         asio::post(strand, [this, session]{
//             sessions.emplace(session);
//         });
//     }

//     void leave(std::shared_ptr<session> session) {
//         asio::post(strand, [this, session]{
//             sessions.erase(session);
//         });
//     }

//     void leave_all() {
//         asio::post(strand, [this]{
//             for(auto& s: sessions)
//                 s->stopping = true;
//         });
//     }

//     void deliver(const std::string& sender_msg, std::shared_ptr<session> sender) {
//         asio::post(strand, [this, sender_msg, sender]{
//             std::stringstream msgs;

//             msgs << sender->get_ep() << ": " << sender_msg << std::endl;

//             std::string msg{msgs.str()};

//             for(const auto& s: sessions) {
//                 if(s != sender) {
//                     s->send(msg);
//                 }
//             }
//         });
//     }

//     void server_deliver(const std::string& msg) {
//         asio::post(strand, [this, msg]{
//             for(const auto& s: sessions)
//                 s->send(msg);
//         });
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
//                 else if(stopping){
//                     close_connection();
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

// class server: public std::enable_shared_from_this<server> {
//     std::shared_ptr<chatroom> chat;
    
//     tcp::acceptor acceptor;
//     int port;

//     asio::io_context& ioc;
//     asio::signal_set sig_set;


//     void do_accept() {
//         acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::cout << "New connection: " << sock.local_endpoint() << std::endl;

//                 auto session_ptr = std::make_shared<session>(std::move(sock), chat);
//                 session_ptr->stopping = false;

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
//         port(port), acceptor(ioc, tcp::endpoint(tcp::v4(), port)), chat(std::make_shared<chatroom>(ioc)), ioc(ioc), sig_set(ioc, SIGINT, SIGTERM)
//         {}

//     void start() {
//         std::cout << "Server running. Press ctrl+c to stop." << std::endl;


//         sig_set.async_wait([this, self = shared_from_this()](system::error_code ec, int){
//             if(!ec) {
//                 std::cout << "\nServer marked as stoped." << std::endl;

//                 acceptor.close();

//                 chat->server_deliver("Server stoped.");
//                 chat->leave_all();
//             }
//         });

//         do_accept();
//     }
// };


// int main() {
//     try {
//         asio::io_context ioc;

//         auto serv{std::make_shared<server>(ioc, 1234)};

//         serv->start();

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cout << e.what() << std::endl;
    
//         return 1;
//     }

//     return 0;
// }
