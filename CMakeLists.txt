cmake_minimum_required(VERSION "3.31.2")
project("iofox")

file(GLOB_RECURSE src "src/*")
add_executable(${PROJECT_NAME} ${src})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

target_include_directories(${PROJECT_NAME} PUBLIC "src")
