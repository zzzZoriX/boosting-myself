cmake_minimum_required(VERSION 3.15)
project(boosting)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost_asio CONFIG REQUIRED)
target_link_libraries(boosting PRIVATE Boost::asio)

add_executable(boosting asio1.cpp)