cmake_minimum_required(VERSION 3.10)
project(OS_lab2_V16)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Настройка для разных платформ
if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-D_WIN32)
    set(PLATFORM_LIBS)
else()
    message(STATUS "Configuring for Linux/Unix")
    set(PLATFORM_LIBS pthread m rt)
endif()

add_executable(monte_carlo 
    src/monte_carlo.c
)

target_include_directories(monte_carlo PRIVATE include)
target_link_libraries(monte_carlo PRIVATE ${PLATFORM_LIBS})

option(BUILD_PERFORMANCE_TEST "Build performance test program" ON)
if(BUILD_PERFORMANCE_TEST)
    add_executable(performance_test 
        src/performance_test.c
    )
    
    target_include_directories(performance_test PRIVATE include)
    target_link_libraries(performance_test PRIVATE ${PLATFORM_LIBS})
endif()

if(UNIX AND NOT APPLE)
    add_custom_command(TARGET monte_carlo POST_BUILD
        COMMAND chmod +x $<TARGET_FILE:monte_carlo>
    )
    if(BUILD_PERFORMANCE_TEST)
        add_custom_command(TARGET performance_test POST_BUILD
            COMMAND chmod +x $<TARGET_FILE:performance_test>
        )
    endif()
endif()

option(ENABLE_DEBUG "Enable debug symbols" OFF)
if(ENABLE_DEBUG)
    target_compile_options(monte_carlo PRIVATE -g -O0)
    if(BUILD_PERFORMANCE_TEST)
        target_compile_options(performance_test PRIVATE -g -O0)
    endif()
else()
    target_compile_options(monte_carlo PRIVATE -O3)
    if(BUILD_PERFORMANCE_TEST)
        target_compile_options(performance_test PRIVATE -O3)
    endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(monte_carlo PRIVATE -Wall -Wextra -Wpedantic)
    if(BUILD_PERFORMANCE_TEST)
        target_compile_options(performance_test PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")