find_package(SDL)
find_library(X11_LIBRARY X11)

if(SDL_LIBRARY AND SDL_INCLUDE_DIR AND X11_LIBRARY AND Xext_LIBRARY)
    set(OS_SPECIFIC)

    if(UNIX AND NOT APPLE)
        set(OS_SPECIFIC sdlX.cpp)
    endif()

    if(UNIX AND APPLE)
        set(OS_SPECIFIC macosx_main.m)
    endif()

    if(WIN32)
        set(OS_SPECIFIC winmain.c)
    endif()

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
        ${OS_SPECIFIC}
        )

    target_link_libraries(front-end-sdl ${SDL_LIBRARY})

    if(UNIX AND NOT APPLE)
        target_link_libraries(front-end-sdl ${X11_LIBRARY})
    endif()

    target_include_directories(front-end-sdl PUBLIC . ${SDL_INCLUDE_DIR})
endif()