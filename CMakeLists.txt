cmake_minimum_required(VERSION 3.10)
project(sum_free_sets)

# For std::filesystem
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
endif(CMAKE_COMPILER_IS_GNUCC)


add_executable(complete_sum_free_sets_rewrite
        complete_sum_free_rewrite.cpp
        )
