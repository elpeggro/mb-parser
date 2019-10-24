cmake_minimum_required(VERSION 3.9)
project(mb_parser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}")

set(MB_PARSER_SRC_FILES
        src/mb_parser.cpp
        src/structs.h
        src/ReferenceGraph)

set(CHAIN_WEIGHTS_SRC_FILES
        src/chains/chain_weights.cpp src/chains/Graph.cpp src/chains/Graph.h src/chains/Vertex.cpp src/chains/Vertex.h)

add_executable(mb_parser ${MB_PARSER_SRC_FILES})
add_executable(chain_weights ${CHAIN_WEIGHTS_SRC_FILES})
