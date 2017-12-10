find_package(SDL2)

if(SDL2_FOUND)
    add_library(front-end-sdl2 host_bltmacros.h sdl2.h sdl2.cpp keycode_map.cpp)

        # workaround for SDL2_LIBRARIES containing trailing whitespace that is illegal in CMake 3.10
    string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

    target_link_libraries(front-end-sdl2 ${SDL2_LIBRARIES})
    target_include_directories(front-end-sdl2
        PUBLIC .
        PRIVATE ${SDL2_INCLUDE_DIRS})
endif()