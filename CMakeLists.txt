cmake_minimum_required(VERSION 3.24.2)
project(graph_matching_with_pthread)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

set(SOURCE_FILES
    main.cpp
    CSRGraph.cpp
    PatternGraph.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(csr_graph STATIC
    CSRGraph.cpp
    CSRGraph.h
)

add_library(pattern_graph STATIC
    PatternGraph.cpp
    PatternGraph.h
)

add_executable(main ${SOURCE_FILES})

target_link_libraries(main
    csr_graph
    pattern_graph
    ${CMAKE_THREAD_LIBS_INIT}
)