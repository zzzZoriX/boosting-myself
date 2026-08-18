cmake_minimum_required(VERSION 3.15)
project(boosting)
project(client)
set(CMAKE_CXX_STANDARD 23)

add_executable(boosting ./asio/asio1.cpp ./asio/asio2.cpp ./asio/asio3.cpp
    asio/asio4.cpp)
add_executable(client ./asio/asynclient.cpp)

find_package(Boost_asio CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED) 

target_link_libraries(boosting PRIVATE Boost::asio OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(client PRIVATE Boost::asio)