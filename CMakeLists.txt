cmake_minimum_required(VERSION 3.5)
project(LSPClient LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Core  CONFIG REQUIRED)

add_library(LSPClient STATIC
    include/LSPClient.hpp
    include/LSP.hpp
    include/LSPUri.hpp

    third_party/nlohmann/json.hpp
    
    src/LSPClient.cpp
)

target_include_directories(LSPClient PUBLIC
    include
    third_party
)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(LSPClient
		PRIVATE
		-ansi
		-pedantic
		-Wall
		-Wextra
		-Woverloaded-virtual
		-Winit-self
		-Wunreachable-code
		-Wno-unused-parameter
	)
endif(CMAKE_COMPILER_IS_GNUCXX)

target_link_libraries(LSPClient
    Qt5::Core
)

