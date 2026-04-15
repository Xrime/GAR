cmake_minimum_required(VERSION 3.20)

project(GAR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(GAR
        src/main.cpp
        include/core/ihttp_transport.h
        include/core/http_client.h
        src/core/http_client.cpp
        include/core/http_request.h
        include/core/http_response.h
)

target_link_libraries(GAR PRIVATE CURL::libcurl)