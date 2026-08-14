cmake_minimum_required(VERSION 3.15)
project(boosting)
project(client)
set(CMAKE_CXX_STANDARD 23)

add_executable(boosting asio1.cpp)
add_executable(client asynclient.cpp)

find_package(Boost_asio CONFIG REQUIRED)
target_link_libraries(boosting PRIVATE Boost::asio)
target_link_libraries(client PRIVATE Boost::asio)