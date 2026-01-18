cmake_minimum_required(VERSION 3.20)
add_executable(full_system_demo main.c)
target_link_libraries(full_system_demo PRIVATE plcopen m)
