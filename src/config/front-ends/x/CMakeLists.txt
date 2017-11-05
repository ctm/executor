find_library(X11_LIBRARY X11)
find_library(Xext_LIBRARY Xext)

if(X11_LIBRARY AND Xext_LIBRARY)

	add_library(front-end-x
			x.cpp
			x_keycodes.cpp
			host_bltmacros.h
			host_vdriver.h
			x.h
			x_keycodes.h
		)
	target_link_libraries(front-end-x ${X11_LIBRARY} ${Xext_LIBRARY})
	target_include_directories(front-end-x PUBLIC .)

endif()