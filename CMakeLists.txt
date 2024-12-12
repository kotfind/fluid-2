cmake_minimum_required(VERSION 3.23)
project(fluid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always")
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=undefined,address,float-divide-by-zero,float-cast-overflow -g3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")

find_program(MOLD_EXECUTABLE mold)
if (NOT ${MOLD_EXECUTABLE} STREQUAL MOLD_EXECUTABLE-NOTFOUND)
    set(CMAKE_CXX_FLAGS "-fuse-ld=mold")
else()
    message(WARNING "mold executable not found")
endif()

find_program(CCACHE_EXECUTABLE ccache)
if (NOT ${CCACHE_EXECUTABLE} STREQUAL CCACHE_EXECUTABLE-NOTFOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
else()
    message(WARNING "ccache executable not found")
endif()

if (DEFINED TYPES)
    add_compile_definitions("TYPES=${TYPES}")
else()
    message(FATAL_ERROR "TYPES are not defined")
endif()

if (DEFINED SIZES)
    add_compile_definitions("SIZES=${SIZES}")
else()
    message(FATAL_ERROR "SIZES are not defined")
endif()

add_executable(fluid)

target_sources(fluid PRIVATE
    main.cpp
    Rnd.cpp
    argv_parse.cpp
)

add_custom_target(fluid-run COMMAND fluid)
