cmake_minimum_required(VERSION 3.15)
project(gensudoku C)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_C_STANDARD 17)

add_executable(gensudoku
        main.c
        utils.h
        sudoku.h sudoku.c
        generator.c generator.h
        field_subset.c field_subset.h
        heuristics.c heuristics.h
        tests.h)
