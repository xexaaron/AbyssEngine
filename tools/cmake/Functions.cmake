# Functions.cmake

function(add_subproject name)
    string(TOUPPER ${name} UPPER_NAME)
    set(${UPPER_NAME}_DIR ${name} PARENT_SCOPE)
    add_subdirectory("Tools/${name}")
    set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/Tools")
    target_compile_options(${name} PRIVATE ${COMPILE_OPTS})
endfunction()

function(set_vendor_properties library folder)
    if(${library} STREQUAL "glfw")
        foreach(target GLFW3 update_mappings uninstall)
            if(TARGET ${target})
                set_target_properties(${target} PROPERTIES FOLDER ${folder})
            endif()
        endforeach()
    endif()

    set_target_properties(${library} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            FOLDER ${folder}
    )
endfunction()