#
# Macro that checks for C++11 support and sets compiler flags
# If C++11 is not present it throws message with MESSAGE_SEVERITY.
# Bear in mind, that for MSVC it's just passing "-std=c++0x" to compiler.
#
# Macro:
#   CheckCXX11(MESSAGE_SEVERITY)
#       MESSAGE_SEVERITY        it's not mandatory and has default value of "STATUS"
#                               this is severity of message if no C++11 was found
#
# Output variables:
#   COMPILER_SUPPORTS_CXX11     ON when compiler supports C++11
#   COMPILER_CXX11_FLAG         compiler flag that adds support for C++11
#
#
# Copyright (c) 2015, Marek Pikuła <marek@pikula.co>
# All rights reserved.
#
# Distributed under the OSI-approved BSD License (the "License") see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the License for more information.
#

macro (CheckCXX11)
    set (COMPILER_SUPPORTS_CXX11 OFF)
    
    if (MSVC)
        # MSVC has C++11 turned on by default
        set (COMPILER_SUPPORTS_CXX11 ON)
        set (COMPILER_CXX11_FLAG "")
    else ()
        include (CheckCXXCompilerFlag)
        if (CMAKE_COMPILER_IS_GNUCXX)
            check_cxx_compiler_flag ("-std=gnu++11" GNUXX11)
        endif ()
        if (NOT GNUXX11)
            check_cxx_compiler_flag ("-std=c++11" CXX11)
            check_cxx_compiler_flag ("-std=c++0x" CXX0X)
        endif ()
        
        if (GNUXX11)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
            set (COMPILER_SUPPORTS_CXX11 ON)
            set (COMPILER_CXX11_FLAG -std=gnu++11)
        elseif (CXX11)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
            set (COMPILER_SUPPORTS_CXX11 ON)
            set (COMPILER_CXX11_FLAG -std=c++11)
        elseif (CXX0X)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
            set (COMPILER_SUPPORTS_CXX11 ON)
            set (COMPILER_CXX11_FLAG -std=c++0x)
        endif ()
    endif ()
    
    if (NOT COMPILER_SUPPORTS_CXX11)
        if (${ARGC})
            set (MESSAGE_SEVERITY ${ARGV0})
        else ()
            set (MESSAGE_SEVERITY "STATUS")
        endif ()
        message (${MESSAGE_SEVERITY} "C++11 support in ${CMAKE_CXX_COMPILER} compiler was not detected. Please use a different C++ compiler.")
        unset (MESSAGE_SEVERITY)
    endif ()
    
    unset (GNUXX11)
    unset (CXX11)
    unset (CXX0X)
endmacro ()
