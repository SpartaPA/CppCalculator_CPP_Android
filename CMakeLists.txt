cmake_minimum_required(VERSION 3.22)
project(CppCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(CppCalculator
    src/main.cpp
    src/calculator_engine.cpp
    src/calculator_engine.h
    src/calculator_window.cpp
    src/calculator_window.h
)

target_link_libraries(CppCalculator PRIVATE Qt6::Widgets)
