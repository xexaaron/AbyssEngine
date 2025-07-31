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


function(make_source_group folder)
    set(pub_source_dir "Source/Public/${folder}")
    set(pub_source_group "Public/${folder}")
    set(private_source_dir "Source/Private/${folder}")
    set(private_source_group "Private/${folder}")

    file(GLOB headers "${pub_source_dir}/*.h")
    source_group("${pub_source_group}" FILES ${headers})

    file(GLOB sources "${private_source_dir}/*.cpp")
    source_group("${private_source_group}" FILES ${sources})
endfunction()

# Usage: make_source_groups(folder, folder, ...)
function(make_source_groups)
    foreach(folder IN LISTS ARGN)
        make_source_group(${folder})
    endforeach()
endfunction()

function(copy_files_post_build TARGET_NAME FILE_LIST)
    foreach(FILE_PATH IN LISTS ${FILE_LIST})
        get_filename_component(FILE_NAME "${FILE_PATH}" NAME)
        if(NOT EXISTS "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${FILE_NAME}"
           AND NOT FILE_NAME STREQUAL "vulkan-1.dll" 
           AND NOT FILE_NAME STREQUAL "vulkan-1.dll.pdb"
        )
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FILE_PATH}"
                    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${FILE_NAME}"
                COMMENT "Copy ${FILE_NAME}"
            )
        endif()
    endforeach()
endfunction()