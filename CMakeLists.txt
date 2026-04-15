cmake_minimum_required(VERSION 3.20)

project(GAR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

find_package(CURL REQUIRED)

add_executable(GAR
        src/main.cpp
        include/core/ihttp_transport.h
        include/core/http_client.h
        src/core/http_client.cpp
        include/core/http_request.h
        include/core/http_response.h
        include/anonymity/tor_connector.h
        src/anonymity/tor_connector.cpp
)

target_link_libraries(GAR PRIVATE CURL::libcurl)

message(STATUS "GAR Build Configuration")
message(STATUS "  CMake Version: ${CMAKE_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source Dir: ${CMAKE_CURRENT_SOURCE_DIR}")