if(WIN32)
add_library(os-win32
            win32.h
            win32.cpp
            win_serial.cpp
            win_serial.h
            win_launch.cpp)
target_link_libraries(os-win32 syn68k front-end-sdl2)
endif()