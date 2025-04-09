# Functions.cmake

function(add_subproject name)
    string(TOUPPER ${name} UPPER_NAME)
    set(${UPPER_NAME}_DIR ${name} PARENT_SCOPE)
    add_subdirectory("Tools/${name}")
    set_target_properties(${name} PROPERTIES FOLDER "${PROJECT_NAME}/Tools")
    target_compile_options(${name} PRIVATE ${COMPILE_OPTS})
endfunction()

function(set_vendor_properties library folder)
    set_target_properties(${library} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}/${CMAKE_BUILD_TYPE}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}/${CMAKE_BUILD_TYPE}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}/${CMAKE_BUILD_TYPE}"
            FOLDER ${folder}
    )
endfunction()