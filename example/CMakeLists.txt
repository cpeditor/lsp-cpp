cmake_minimum_required(VERSION 3.5)
project(LSPClientExample LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Core  CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
add_executable(LSPClientExample main.cpp mainwindow.cpp mainwindow.hpp)

target_include_directories(LSPClientExample PUBLIC
    ../include
    mainwindow.hpp
)

target_link_libraries(LSPClientExample
    Qt5::Core
    Qt5::Widgets
    LSPClient
)

