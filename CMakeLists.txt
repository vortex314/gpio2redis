cmake_minimum_required(VERSION 3.0.0)
project(gpio2redis VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g ")
add_definitions(-DLINUX -std=c++11 -DHAS_GPIO)

set(LIMERO limero)
set(HIREDIS hiredis )
set(ARDUINOJSON ArduinoJson)

add_executable(gpio2redis 
        src/gpio2redis.cpp 
        ${LIMERO}/linux/Gpio.cpp
        ${LIMERO}/src/StringUtility.cpp
        ${LIMERO}/src/printf.c
        ${LIMERO}/linux/Redis.cpp
        ${LIMERO}/linux/ConfigFile.cpp
        ${LIMERO}/linux/LogFile.cpp
        ${LIMERO}/linux/Log.cpp
        ${LIMERO}/linux/Sys.cpp
        ${LIMERO}/linux/limero.cpp
        ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(GCC_FLAGS "-DLINUX")
include(CPack)

add_definitions(${GCC_FLAGS})
include_directories(src  
        limero/inc 
        limero/linux 
        ArduinoJson/src 
        ${HIREDIS}
        )
target_link_libraries(gpio2redis 
        -lpthread 
        -L../${HIREDIS} -l:libhiredis.a
        -lrt 
        -pthread 
        -latomic )
EXECUTE_PROCESS( COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE )

if ( ${ARCH} STREQUAL "armv6l" OR ${ARCH} STREQUAL "armv7l")
        target_link_libraries(gpio2redis -lwiringPi)
	set(GCC_FLAGS "-DHAS_GPIO")
endif()

