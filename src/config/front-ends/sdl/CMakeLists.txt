find_package(SDL)

find_library(X11_LIBRARY X11)

add_library(front-end-sdl
    sdlevents.cpp
    sdl_mem.cpp
    sdlquit.cpp
    sdlscrap.cpp
    sdlwin.cpp
    sdlwm.cpp
    syswm_map.cpp
    for_sam.h
    host_bltmacros.h
    host_vdriver.h
    map.h
    sdl.h
    SDL_bmp.h
    sdl_mem.h
    sdlevents.h
    sdlk_to_mkv.h
    sdlquit.h
    sdlscrap.h
    sdlX.h
    syswm_map.h
    syswm_vars.h

    sdlX.cpp
        )
target_link_libraries(front-end-sdl ${SDL_LIBRARY} ${X11_LIBRARY})

target_include_directories(front-end-sdl PUBLIC . ${SDL_INCLUDE_DIR})
