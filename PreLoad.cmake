## Fix for windows
if (WIN32)
	set (CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
endif ()
