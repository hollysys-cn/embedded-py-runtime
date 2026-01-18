cmake_minimum_required(VERSION 3.20)
add_executable(pid_control_demo main.c)
target_link_libraries(pid_control_demo PRIVATE plcopen m)
