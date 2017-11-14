find_package(SDL2)

if(SDL2_FOUND)
add_library(front-end-sdl2 host_bltmacros.h sdl2.h sdl2.cpp)

target_link_libraries(front-end-sdl2 ${SDL2_LIBRARIES})
target_include_directories(front-end-sdl2
    PUBLIC .
    PRIVATE ${SDL2_INCLUDE_DIRS})

endif()