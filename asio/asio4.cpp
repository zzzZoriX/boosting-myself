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
// class tsl_session: public std::enable_shared_from_this<tsl_session> {
//     asio::streambuf buffer;
//     ssl::stream<tcp::socket> stream;

//     asio::strand<asio::any_io_executor> strand;


//     void shutdown() {
//         stream.async_shutdown(asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec){
//             if(!ec) {
//                 std::cout << "Client " << stream.lowest_layer().remote_endpoint() << " disconnected" << std::endl;
                
//                 system::error_code close_ec;
//                 stream.lowest_layer().close(close_ec);
//             }
//         }));
//     }

//     void read() {
//         asio::async_read_until(
//             stream,
//             buffer,
//             "\n",
//             asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
//                 if(!ec) {
//                     std::cout << "new message received" << std::endl;

//                     write(len);
//                 }
//                 else {
//                     std::cout << ec.message() << std::endl;

//                     shutdown();
//                 }
//             })
//         );
//     }

//     void write(std::size_t bytes) {
//         asio::async_write(
//             stream,
//             buffer,
//             asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t){
//                 if(!ec) {
//                     read();
//                 }
//                 else {
//                     std::cout << ec.message() << std::endl;

//                     shutdown();
//                 }
//             })
//         );
//     }

// public:
//     tsl_session(tcp::socket&& sock, ssl::context& ctx, asio::io_context& ioc):
//         stream(std::move(sock), ctx), strand(asio::make_strand(ioc)) {}
        
//     void start() {
//         stream.async_handshake(
//             ssl::stream_base::server,
//             [this, self = shared_from_this()](system::error_code ec){
//                 if(!ec) {
//                     read();
//                 }
//                 else {
//                     shutdown();
//                 }
//             }
//         );
//     }
// };

// class tsl_server {
//     asio::io_context& ioc;
//     tcp::acceptor acceptor;
//     ssl::context& ctx;

//     const int port;


//     void do_accept() {
//         acceptor.async_accept([this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::cout << "new connection from " << sock.remote_endpoint() << std::endl;

//                 std::make_shared<tsl_session>(std::move(sock), ctx, ioc)->start();
//             }
//             else {
//                 std::cout << ec.message() << std::endl;
//             }

//             do_accept();
//         });
//     }

// public:
//     tsl_server(asio::io_context& ioc, const int port, ssl::context& ctx):
//         port(port), ioc(ioc), ctx(ctx), acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {}

//     void start() {
//         std::cout << "server running on 127.0.0.1:" << port << std::endl;

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

//         tsl_server serv{ioc, 1234, ctx};
//         serv.start();

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cerr << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 4.2 Парсер команд -- выполнено
// class commands_processor;

// class command: public std::enable_shared_from_this<command> {
//     tcp::socket sock;
//     asio::streambuf buffer;

//     asio::strand<asio::any_io_executor> strand;

//     std::weak_ptr<commands_processor> cmd_proc;


//     void read();

// public:
//     command(asio::io_context& ioc, tcp::socket&& sock, std::shared_ptr<commands_processor> cmd_proc): 
//         sock(std::move(sock)), strand(asio::make_strand(ioc)), cmd_proc(cmd_proc) {}

//     void start() {
//         read();
//     }

//     void send(std::string msg) {
//         asio::async_write(
//             sock,
//             asio::buffer(msg),
//             asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t){
//                 if(ec) {
//                     std::cout << ec.message() << std::endl;
//                 }
//             })
//         );
//     }
// };

// class commands_processor: public std::enable_shared_from_this<commands_processor> {
//     tcp::acceptor acceptor;
//     const int port;

//     asio::strand<asio::any_io_executor> strand;

//     asio::io_context& ioc;


//     void do_accept() {
//         acceptor.async_accept(asio::bind_executor(strand, [this](system::error_code ec, tcp::socket sock){
//             if(!ec) {
//                 std::cout << "new connection" << std::endl;

//                 std::make_shared<command>(ioc, std::move(sock), shared_from_this())->start();
                
//                 do_accept();
//             }
//             else {
//                 std::cout << ec.message() << std::endl;
//             }
//         }));
//     }

// public:
//     commands_processor(asio::io_context& ioc, const int port):
//         strand(asio::make_strand(ioc)), port(port), acceptor(ioc, tcp::endpoint(tcp::v4(), port)), ioc(ioc) {}

//     void start() {
//         do_accept();
//     }

//     void process_command(std::shared_ptr<command> cmd_ptr, std::string command) {
//         std::cout << "command received: " << std::move(command);

//         while (!command.empty() && 
//           (command.back() == '\r' || 
//            command.back() == '\n' || 
//            command.back() == ' '  || 
//            command.back() == '\t')) 
//         {
//             command.pop_back();
//         }

//         if(command == "QUIT" || command == "QUIT\r") {
//             asio::post(strand, [this, self = shared_from_this()]{ 
//                 system::error_code close_ec;
//                 acceptor.close(close_ec);

//                 ioc.stop();
//             });
//         }
//         else if(command == "PING" || command == "PING\r") {
//             asio::post(strand, [this, cmd_ptr, self = shared_from_this()]{
//                 cmd_ptr->send("PONG");
//             });
//         }
//     }
// };

// void command::read() {
//     asio::async_read_until(
//         sock,
//         buffer,
//         "\n",
//         asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t len) {
//             if(!ec) {
//                 if(auto cmd_proc_ptr = cmd_proc.lock())
//                     cmd_proc_ptr->process_command(shared_from_this(), std::string(asio::buffers_begin(buffer.data()), asio::buffers_begin(buffer.data()) + len));

//                 buffer.consume(len);

//                 read();
//             }
//             else {
//                 std::cout << ec.message() << std::endl;
//             }
//         })
//     );
// }


// int main() {
//     try {
//         asio::io_context ioc;

//         auto proc = std::make_shared<commands_processor>(ioc, 1234);
//         proc->start();

//         ioc.run();
//     }
//     catch(const std::exception& e) {
//         std::cerr << e.what() << std::endl;

//         return 1;
//     }

//     return 0;
// }

// 4.3 -- мини-тест
/*

1.  Почему использование глобальной фиксированной арены памяти (например, std::array<char, 1024> внутри сессии) 
    безопасна для комбинации async_read -> async_write, но может вызвать инвалидацию/ошибку, если одновременно запустить и 
    async_read, и async_write на одной и той же сессии?
    --  потому что произойдет параллельное обращение к одной памяти, что может вызвать sigseg.
    ?-  5/10
    !-  Декапсуляция арены памяти происходит за счет флага занятости (например, bool in_use_). Если async_read и 
        async_write запускаются одновременно, первый займет арену, а второй не поместится в нее (или перезапишет служебный блок) 
        и вынужден будет либо фолбэкнуться в heap, либо затереть память хэндлера первой операции. Ошибка будет не только в SIGSEGV, 
        сколько в затирании служебных данных Asio completion handler'а (Memory Corruption).

2.  В какой момент Asio возвращает память аллокатору через deallocate — до вызова завершающего completion-хэндлера 
    (твоей лямбды) или строго после его выполнения?
    --  строго после.
    ?-  0/10
    !-  СТРОГО ДО вызова твоего completion-хэндлера. Asio извлекает твою лямбду из выделенного блока памяти, 
        освобождает память (вызывает deallocate), и ТОЛЬКО ПОТОМ запускает тело самой лямбды. Именно поэтому внутри лямбды 
        локальная арена памяти уже считается свободной, и из этой же лямбды можно сразу же запускать следующий async_read, 
        повторно задействуя ту же самую арену памяти на стеке без новых аллокаций!

*/