project(Executor)
cmake_minimum_required(VERSION 3.9)

if( NOT EXISTS "${CMAKE_SOURCE_DIR}/syn68k/.git" )
    message( FATAL_ERROR
        "The git submodule 'syn68k' is not available. Please run\n"
        "git submodule update --init --recursive" )
endif()

add_subdirectory(syn68k)
add_subdirectory(src)

