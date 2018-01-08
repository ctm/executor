find_package(Qt5Widgets)

if(TARGET Qt5::Widgets)
    add_library(front-end-qt
            host_vdriver.h
            qt.h
            qt.cpp
        )

    target_link_libraries(front-end-qt syn68k Qt5::Widgets)
    target_include_directories(front-end-qt
            PUBLIC .)
endif()
