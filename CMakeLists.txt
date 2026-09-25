cmake_minimum_required(VERSION 3.16)

project(url_shortener
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Tell Crow where standalone Asio is installed
set(ASIO_INCLUDE_DIR
    "C:/msys64/ucrt64/include"
    CACHE PATH "Asio include directory"
)

# Download Crow
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3.3
)

FetchContent_MakeAvailable(Crow)

# SQLite
set(SQLITE3_INCLUDE_DIR
    "C:/msys64/ucrt64/include"
)

set(SQLITE3_LIBRARY
    "C:/msys64/ucrt64/lib/libsqlite3.a"
)

# Main application
add_executable(url_shortener
    src/main.cpp
    src/database.cpp
)

target_include_directories(url_shortener
    PRIVATE
    include
    ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(url_shortener
    PRIVATE
    Crow::Crow
    ${SQLITE3_LIBRARY}
    ws2_32
    mswsock
)