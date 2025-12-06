cmake_minimum_required(VERSION 3.10)
project(OS_lab1_V16)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-D_WIN32)
    set(PLATFORM_SOURCES )
    set(PLATFORM_LIBS)
else()
    message(STATUS "Configuring for Linux/Unix")
    set(PLATFORM_SOURCES src/os_linux.c)
    set(PLATFORM_LIBS)
endif()

add_executable(parent src/parent.c ${PLATFORM_SOURCES})
target_include_directories(parent PRIVATE include)

add_executable(child src/child.c)
target_include_directories(child PRIVATE include)