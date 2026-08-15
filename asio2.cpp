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


int main() {
    try {
        asio::io_context ioc;

        asio::strand strand = asio::make_strand(ioc);

        int shared_counter = 0;

        for (int i = 0; i < 5; ++i) {
            asio::post(strand, [&shared_counter, i](){
                std::cout << "Task " << i << "Task ID " << std::this_thread::get_id() << std::endl;

                ++shared_counter;
            });
        }

        std::vector<std::thread> threads;
        
        for(int i = 0; i < 4; ++i)
            threads.emplace_back([&ioc](){
                ioc.run();
            });

        for(auto& t: threads) t.join();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;

        return 1;
    }

    return 0;
}