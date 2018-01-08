if(APPLE AND UNIX)
add_library(os-macosx
            macosx.h
            macosx.mm)
find_library(COCOA_LIBRARY Cocoa)
target_link_libraries(os-macosx ${COCOA_LIBRARY})

endif()
