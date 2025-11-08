cmake_minimum_required(VERSION 3.16)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

add_executable(MyProgram)

target_sources(MyProgram
  PRIVATE
    src/main.cpp 
)

set_target_properties(MyProgram PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)