cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(clinic 
               src/utils.cpp
               src/data.cpp
               src/main.cpp
)