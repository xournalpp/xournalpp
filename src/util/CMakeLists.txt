# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB util_SOURCES
        logger/**.cpp
        serializing/**.cpp
        *.cpp)

if (ENABLE_AUDIO)
    file (GLOB util_audio_SOURCES audio/**.cpp)
    set(util_SOURCES ${util_SOURCES} ${util_audio_SOURCES})
endif (ENABLE_AUDIO)

add_library (util STATIC ${util_SOURCES})
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
