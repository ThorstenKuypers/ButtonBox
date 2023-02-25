set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-gcc)          
set(CINCS "-IC:/msys64/mingw32/avr/include")
set(CXXFLAGS "-g -Os -Wall -Wextra -Wpedantic -fpermissive -fno-exceptions fno-threadsafe-statics --param=min-pagesize=0 -mmcu=atmega8u2 F_CPU=16000000UL")
set(CMAKE_CXX_FLAGS ${CXXFLAGS})
#set(AVR_LINKER_LIBS)
#set(CMAKE_LINKFL)