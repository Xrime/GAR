cmake_minimum_required(VERSION 3.20)

project(GAR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include_directories(include)

find_package(CURL REQUIRED)

add_executable(GAR
        src/main.cpp
        src/core/http_client.cpp
        src/anonymity/tor_connector.cpp
        src/anonymity/tor_manager.cpp
)

target_link_libraries(GAR PRIVATE CURL::libcurl)

if(WIN32)
    target_link_libraries(GAR PRIVATE ws2_32)

endif ()

# Add this near your other target settings

message(STATUS "GAR Build Configuration")
message(STATUS "  CMake Version: ${CMAKE_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: Windows")
message(STATUS "  Linking: CURL + ws2_32 (Windows sockets)")