#include(target_8u2.cmake)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ./target_8u2.cmake)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CINCS "-IC:/msys64/mingw32/avr/include")
set(CXXFLAGS "${CINCS} -g -Os -Wall -Wextra -Wpedantic -fpermissive -fno-exceptions fno-threadsafe-statics --param=min-pagesize=0 -mmcu=atmega8u2 F_CPU=16000000UL")
set(CMAKE_CXX_FLAGS ${CXXFLAGS})

project(duino-hid
        LANGUAGES CXX
        VERSION 0.1
        DESCRIPTION "Test implementation to use Arduino as USB HID device")       
add_library(duino-hid STATIC
                inc/*.hpp
                src/main.cpp
                src/hal_m8u2.cpp
                src/hal.cpp
                src/RingBuffer.cpp
                src/timer.cpp
                src/Usart.cpp)   
target_compile_features(duino-hid PRIVATE cxx_std_17)
# target_compile_features(duino-hid PRIVATE 
#                         -g
#                         -Os
#                         -Wall
#                         -Wextra
#                         -Wpedantic
#                         -fpermissive
#                         -fno-exceptions
#                         -fno-threadsafe-statistics
#                         --param=min-pagesize=0
#                         -mmcu=atmega8u2
#                         F_CPU=16000000UL        
#                         )
set(CMAKE_MAKE_PROGRAM make)
