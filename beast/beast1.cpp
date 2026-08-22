#include <iostream>
#include <boost/asio/ssl.hpp>
#include <array>
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::beast;
using namespace http;
using namespace websocket;
using ip::tcp;


int main() {
    try {
        const std::string host{"httpbin.org"};
        const std::string port{"443"};

        io_context ioc;
        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_default_verify_paths();

        ssl::stream<tcp::socket> stream{ioc, ctx};

        if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::beast::system_error(ec);
        }

        tcp::resolver resolver{ioc};

        const auto results{resolver.resolve(host, port)};
        connect(stream.next_layer(), results);

        stream.handshake(ssl::stream_base::client);

        request<string_body> req{verb::get, "/get", 11};
        req.set(field::host, host);
        req.set(field::user_agent, BOOST_BEAST_VERSION_STRING);

        boost::beast::http::write(stream, req);

        flat_buffer buffer;

        response<string_body> res{};

        boost::beast::http::read(stream, buffer, res);

        
        std::cout << res.result_int() << std::endl;
        std::cout << res.body() << std::endl;

        error_code ec;
        stream.shutdown(ec);
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;

        return 1;
    }

    return 0;
}