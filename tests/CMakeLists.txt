# ── Test targets for icetea ───────────────────────────────────────────────

function(icetea_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE icetea gtest gtest_main)
    set_target_properties(${name} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/code/bin"
    )
    add_test(NAME ${name} COMMAND ${name})

    # Copy icetea.dll next to test exe (Windows)
    if(WIN32 AND TARGET icetea)
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:icetea> $<TARGET_FILE_DIR:${name}>
            COMMENT "[${name}] Copying icetea.dll"
        )
    endif()
endfunction()

icetea_add_test(icetea_types_test          types_test.cpp)
icetea_add_test(icetea_icu4x_bridge_test   icu4x_bridge_test.cpp)
icetea_add_test(icetea_unicode_utils_test  unicode_utils_test.cpp)
