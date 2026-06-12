cmake_minimum_required(VERSION 3.20)

project(GAR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# 1. Fetch FTXUI directly from GitHub for the interactive UI
include(FetchContent)
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG main
)
FetchContent_MakeAvailable(ftxui)

include_directories(include)
include_directories("c:/msys64/mingw64/include")

# 2. Find core dependencies
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# 3. Common backend sources shared between both apps
set(GAR_SOURCES
        src/core/http_client.cpp
        src/anonymity/tor_connector.cpp
        src/anonymity/tor_manager.cpp
        src/anonymity/secure_memory.cpp
        src/security/header_analyzer.cpp
        src/anonymity/tor_control.cpp
        src/core/dns_resolver.cpp
        src/anonymity/fingerprint.cpp
        src/security/tls_inspector.cpp
        src/security/request_replay.cpp
)

# 4. Target 1: The original CLI version
add_executable(GAR src/main.cpp src/ui/terminal_ui.cpp src/ui/renderer.cpp ${GAR_SOURCES})
target_link_libraries(GAR PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto C:/msys64/mingw64/lib/libgumbo.a)

# 5. Target 2: The New Interactive TUI version
add_executable(GAR_TUI src/ui/tui_main.cpp src/ui/tui_app.cpp src/ui/settings.cpp ${GAR_SOURCES})
target_link_libraries(GAR_TUI PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto C:/msys64/mingw64/lib/libgumbo.a ftxui::screen ftxui::dom ftxui::component)

# 6. Windows specific networking libraries
if(WIN32)
    target_link_libraries(GAR PRIVATE ws2_32)
    target_link_libraries(GAR_TUI PRIVATE ws2_32)
endif()

# 7. Copy certificates to the build directory for both targets
add_custom_command(TARGET GAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/certs $<TARGET_FILE_DIR:GAR>/certs)

add_custom_command(TARGET GAR_TUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/certs $<TARGET_FILE_DIR:GAR_TUI>/certs)

message(STATUS "GAR Build Configuration Complete")
message(STATUS "  Building Targets: GAR (CLI), GAR_TUI (Interactive)")
message(STATUS "  Libraries: CURL, OpenSSL, Gumbo, FTXUI, ws2_32")