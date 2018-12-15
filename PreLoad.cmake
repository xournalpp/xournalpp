## Fix for windows
if (WIN32)
	set (CMAKE_GENERATOR "MSYS Makefiles" CACHE INTERNAL "" FORCE)
endif ()
