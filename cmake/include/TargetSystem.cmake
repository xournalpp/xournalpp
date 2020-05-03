

add_library(OSInterface INTERFACE)
target_compile_options(OSInterface INTERFACE
        $<$<AND:$<CXX_COMPILER_ID:GNU>, $<PLATFORM_ID:platform_ids>>:-mwindows>
        )

#[[if (WIN32)
    # optimize for size (the Windows .exe is really big)
    set(xournalpp_LDFLAGS ${xournalpp_LDFLAGS} "-Os -s")
elseif (APPLE OR LINUX)
    # Nothing to do for APPLE and linux
else ()
    set(xournalpp_LDFLAGS ${xournalpp_LDFLAGS} -rdynamic)
endif ()]]

add_library(CompilerInterface INTERFACE)
target_compile_definitions(CompilerInterface INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX WIN32_LEAN_AND_MEAN>
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:>
        $<$<CXX_COMPILER_ID:GNU>:>
        )
