find_library(X11_LIBRARY X11)
find_library(Xext_LIBRARY Xext)

if(X11_LIBRARY AND Xext_LIBRARY)

	add_library(front-end-x
			x.cpp
			x_keycodes.cpp
			host_vdriver.h
			x.h
            x_keycodes.h
            sigio_multiplex.h
            sigio_multiplex.cpp
		)
	target_link_libraries(front-end-x syn68k ${X11_LIBRARY} ${Xext_LIBRARY})
	target_include_directories(front-end-x PUBLIC .)
    if(CMAKE_COMPILER_IS_GNUCXX)
        # X11 front-end is legacy code.
        # it doesn't pay to fix this.
        target_compile_options(front-end-x PRIVATE -Wno-write-strings)
    endif()
endif()