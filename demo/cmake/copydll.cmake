macro(CopyDLL target_name)
    if (WIN32)
        add_custom_command(
            TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $CACHE{SDL2_DYNAMIC_LIB_DIR}/SDL2.dll $<TARGET_FILE_DIR:${target_name}>)
    endif()
endmacro(CopyDLL)
