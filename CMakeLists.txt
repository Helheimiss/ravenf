cmake_minimum_required(VERSION 4.1)
project(ravenf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)


add_library(${PROJECT_NAME} SHARED src/main.cpp src/imposter.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)


target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-O3>
)

target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-s>
        $<$<CONFIG:Release>:-static>
)