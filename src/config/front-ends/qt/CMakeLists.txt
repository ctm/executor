find_package(Qt5Gui)

if(TARGET Qt5::Gui)
    add_library(front-end-qt
            host_vdriver.h
            qt.h
            qt.cpp
            available_geometry.h
            available_geometry.cpp
            ../x/x_keycodes.cpp
        )

    target_link_libraries(front-end-qt syn68k Qt5::Gui)
    target_include_directories(front-end-qt
            PUBLIC .)
endif()
