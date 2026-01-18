cmake_minimum_required(VERSION 3.20)
add_executable(filter_demo main.c)
target_link_libraries(filter_demo PRIVATE plcopen m)
