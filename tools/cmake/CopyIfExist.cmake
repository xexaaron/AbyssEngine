# Usage: cmake -P CopyIfExist.cmake "path/to/source" "path/to/destination"

if (DEFINED CMAKE_ARGV4 AND DEFINED CMAKE_ARGV5)
    if (EXISTS "${CMAKE_ARGV4}")
        message(STATUS "Copying ${CMAKE_ARGV4} to ${CMAKE_ARGV5}")
        file(COPY "${CMAKE_ARGV4}" DESTINATION "${CMAKE_ARGV5}")
    else()
        message(WARNING "File ${CMAKE_ARGV4} does not exist, skipping copy.")
    endif()
else()
    message(FATAL_ERROR "cmake -P CopyIfExist.cmake \"path/to/source\" \"path/to/destination\"")
endif()