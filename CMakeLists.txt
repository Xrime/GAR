cmake_minimum_required(VERSION 3.20)

project(GAR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include_directories(include)

find_package(CURL REQUIRED)
find_library(GUMBO_LIBRARY gumbo)
include_directories("c:/msys64/mingw64/include")



add_executable(GAR
        src/main.cpp
        src/core/http_client.cpp
        src/anonymity/tor_connector.cpp
        src/anonymity/tor_manager.cpp
        include/ui/terminal_ui.h
        src/ui/terminal_ui.cpp
        include/anonymity/secure_memory.h
        src/anonymity/secure_memory.cpp
        include/ui/renderer.h
        src/ui/renderer.cpp
        include/security/header_analyzer.h
        src/security/header_analyzer.cpp
        include/anonymity/tor_control.h
        src/anonymity/tor_control.cpp
        include/core/dns_resolver.h
        src/core/dns_resolver.cpp
        include/anonymity/fingerprint.h
        src/anonymity/fingerprint.cpp
        include/security/tls_inspector.h
        src/security/tls_inspector.cpp
)
add_custom_command(TARGET GAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/certs
        $<TARGET_FILE_DIR:GAR>/certs)

target_link_libraries(GAR PRIVATE CURL::libcurl)
target_link_libraries(GAR PRIVATE C:/msys64/mingw64/lib/libgumbo.a)

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