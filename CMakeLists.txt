cmake_minimum_required(VERSION 3.15)
project(boosting)
project(client)
project(beast)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(boosting ./asio/asio1.cpp ./asio/asio2.cpp ./asio/asio3.cpp
    asio/asio4.cpp
    asio/asio5_finale.cpp)
add_executable(client ./asio/asynclient.cpp)
add_executable(beast ./beast/beast1.cpp)

find_package(Boost_asio CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED) 
find_package(Boost_beast CONFIG REQUIRED)

target_link_libraries(boosting PRIVATE Boost::asio OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(client PRIVATE Boost::asio)
target_link_libraries(beast PRIVATE Boost::asio OpenSSL::SSL OpenSSL::Crypto Boost::beast)