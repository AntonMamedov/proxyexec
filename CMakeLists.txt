cmake_minimum_required(VERSION 3.21)
project(proxyexec C)

set(CMAKE_C_STANDARD 99)

include_directories(/usr/src/linux-headers-5.15.0-27/include)
include_directories(/usr/src/linux-headers-5.15.0-27/arch/ia64/include)
include_directories(/usr/src/linux-headers-5.15.0-27/arch/x86/include)
add_executable(proxyexec proxyexec.c)
