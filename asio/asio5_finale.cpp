// this is file for the final test of asio

// part 1 -- Найди UB, Data Race и архитектурные баги
/* 1.1
---------------- code -----------------
class chat_session : public std::enable_shared_from_this<chat_session> {
    tcp::socket socket_;
    asio::strand<asio::any_io_executor> strand_;
    std::string write_queue_;

public:
    chat_session(tcp::socket socket, asio::io_context& ioc)
        : socket_(std::move(socket)), strand_(asio::make_strand(ioc)) {}

    void send_message(const std::string& msg) {
        // Вызывается из ВНЕШНИХ потоков приложения
        write_queue_ += msg + "\n";
        
        asio::async_write(socket_, asio::buffer(write_queue_),
            asio::bind_executor(strand_, 
                [this, self = shared_from_this()](sys::error_code ec, std::size_t) {
                    if (!ec) {
                        write_queue_.clear();
                    }
                }
            )
        );
    }
};
---------------- code -----------------
---------------- answer ---------------
1. строка 12, потому что - обращение к общему буфферу не обернуто в asio::post(), и может произойти ошибка сегментации
---------------- answer ---------------
---------------- score ----------------

---------------- score ----------------
*/

/* 1.2 
---------------- code -----------------
class secure_worker : public std::enable_shared_from_this<secure_worker> {
    ssl::stream<tcp::socket> stream_;
    asio::streambuf buf_;

public:
    secure_worker(tcp::socket sock, ssl::context& ctx) 
        : stream_(std::move(sock), ctx) {}

    void start() {
        // Хэндшейк асинхронный
        stream_.async_handshake(ssl::stream_base::server, 
            [](const sys::error_code& ec) {
                // Если нет ошибки, просто идем дальше
            }
        );

        // Сразу взводим чтение
        asio::async_read_until(stream_, buf_, "\r\n",
            [this, self = shared_from_this()](sys::error_code ec, std::size_t bytes) {
                if (!ec) {
                    process_payload(bytes);
                }
            }
        );
    }

    void process_payload(std::size_t len) {
        std::string data(asio::buffers_begin(buf_.data()), 
                         asio::buffers_begin(buf_.data()) + len);
        // Забыли сделать buf_.consume(len) - допустим. Что ТЯЖЕЛЕЕ произойдет с buf_?
    }
};
---------------- code -----------------
---------------- answer ---------------
1. весь класс. нету strand, что асинхронки обрабатывались в одном потоке
2. на строке 29. может произойти повреждение данных
---------------- answer ---------------
---------------- score ----------------

---------------- score ----------------
*/


// part 2 -- Теория и системный уровень
/* 2.1
    Как Boost.Asio эмулирует Proactor-модель поверх epoll в Linux (что происходит на уровне системных вызовов при async_read)?
    -- asio регестрирует сокеты в epool, и когда в них появляются данные - epool сообщает об этом асинхронке

    Почему истинный Proactor через io_uring кардинально снижает оверхед по Context Switches и Syscalls по сравнению с epoll?
    -- потому что io_uring работает с кольцевым буффером. то есть, кладет операции в буффер, а после выполнения операций ОС, она возвращает результат обратно в буффер
*/
/* 2.2
    Если Thread A выполняет хэндлер внутри strand_1 и вызывает asio::dispatch(strand_2, fn), а Thread B в этот же 
    момент внутри strand_2 делает asio::dispatch(strand_1, fn2) — возможен ли здесь Deadlock? Почему? Чем поведение asio::dispatch отличается от asio::post в этом сценарии?
    -- чем поведение asio::dispatch отличается от asio::post? - диспатч открепляет от strand задачу, а пост наоборот прикрепляет
    -- возможен ли дедлок? - да, потому что идет цикличный вызов.
*/
/* 2.3 
    Если твой сервер закрывает TCP-сокет методом socket.close() во время того, как у клиента в ОС ещё лежат неотправленные 
    или невычитанные данные в сокетном буфере — какой TCP-пакет отправляет ядро Linux клиенту (FIN или RST) и к какой ошибке на стороне клиента это приводит?
    -- сервер отправит FIN. А ошибка будет такова - у клиента не освободится память под сокетный буффер, произойдет утечка.
*/

// part 3 -- Практика — Проектирование модуля
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

class timed_session: public std::enable_shared_from_this<timed_session> {
    tcp::socket sock;
    asio::strand<asio::any_io_executor> strand;

    asio::steady_timer timer;
    int timeout;

    asio::streambuf buffer;


    void read() {
        timer.expires_after(std::chrono::seconds(timeout));
        timer.async_wait(asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec){
            if(!ec) {
                std::cout << "Client disconnected. Timeout" << std::endl;
            }
            else if (ec && ec != asio::error::operation_aborted) {
                std::cout << "Client disconnected by timeout, but error occured: " << ec.message() << std::endl;
            }

            close_connection();
        }));

        asio::async_read_until(
            sock,
            buffer,
            "\n",
            asio::bind_executor(strand, [this, self = shared_from_this()](system::error_code ec, std::size_t len){
                if(!ec) {
                    timer.cancel();

                    std::cout << "New message received from " << sock.remote_endpoint() << std::endl;

                    write(len);
                }
                else {
                    std::cout << "Error occured while server trying receive the message from client " 
                            << sock.remote_endpoint() << ": " << ec.message() << std::endl;

                    close_connection();
                }
            })
        );
    }

    void write(std::size_t len) {
        asio::async_write(
            sock,
            buffer,
            asio::bind_executor(strand, [len, this, self = shared_from_this()](system::error_code ec, std::size_t){
                if(!ec) {
                    buffer.consume(len);

                    std::cout << "Echo was sended correct" << std::endl;

                    read();
                }
                else {
                    std::cout << "Error occured while message sending: " << ec.message() << std::endl;

                    close_connection();
                }
            })
        );
    }

    void close_connection() {
        asio::post(
            strand, 
            [this, self = shared_from_this()]{ 
                system::error_code close_ec; 
                sock.close(close_ec); 
            }
        );
    }

public:
    timed_session(asio::io_context& ioc, const int timeout, tcp::socket&& sock):
        timeout(timeout), timer(ioc), strand(asio::make_strand(ioc)), sock(std::move(sock)) {}

    void start() {
        read();
    }
};



// итоговая - 4.4/10 
// хуево