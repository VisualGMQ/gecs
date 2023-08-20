if (NOT TARGET SDL2_mixer)
    if (NOT SDL2_MIXER_ROOT)
        set(SDL2_MIXER_ROOT "" CACHE PATH "SDL2_mixer root directory")
    endif()
    if (WIN32)  # Windows
        IsMSVCBackend(is_msvc_backend)
        IsMinGWBackend(is_mingw_backend)
        IsX64Compiler(is_x64_compiler)
        if (${is_msvc_backend}) # use MSVC
            set(SDL_MIXER_INCLUDE_DIR "${SDL2_MIXER_ROOT}/include")
            if (${is_x64_compiler})
                set(SDL_MIXER_LIB_DIR "${SDL2_MIXER_ROOT}/lib/x64")
                set(SDL2_MIXER_DYNAMIC_LIB_DIR "${SDL2_MIXER_ROOT}/lib/x64" CACHE PATH "SDL2_mixer directory" FORCE)
            else()
                set(SDL_MIXER_LIB_DIR "${SDL2_MIXER_ROOT}/lib/x86")
                set(SDL2_MIXER_DYNAMIC_LIB_DIR "${SDL2_MIXER_ROOT}/lib/x86" CACHE PATH "SDL2_mixer directory" FORCE)
            endif()
            set(LIB_PATH "${SDL_MIXER_LIB_DIR}/SDL2_mixer.lib")
            set(DYNAMIC_LIB_PATH "${SDL_MIXER_LIB_DIR}/SDL2_mixer.dll")

            add_library(SDL2::MIXER SHARED IMPORTED GLOBAL)
            set_target_properties(
                SDL2::MIXER
                PROPERTIES
                    IMPORTED_LOCATION ${DYNAMIC_LIB_PATH}
                    IMPORTED_IMPLIB ${LIB_PATH}
                    INTERFACE_INCLUDE_DIRECTORIES ${SDL_MIXER_INCLUDE_DIR}
            )
            add_library(SDL2_mixer INTERFACE IMPORTED GLOBAL)
            target_link_libraries(SDL2_mixer INTERFACE SDL2::MIXER)
        elseif (${is_mingw_backend}) # use MINGW
            if (${is_x64_compiler})
                set(SDL_MIXER_INCLUDE_DIR "${SDL2_MIXER_ROOT}/x86_64-w64-mingw32/include/SDL2")
                set(SDL_MIXER_LIB_DIR "${SDL2_MIXER_ROOT}/x86_64-w64-mingw32/lib")
                set(SDL2_MIXER_DYNAMIC_LIB_DIR "${SDL2_MIXER_ROOT}/x86_64-w64-mingw32/bin" CACHE PATH "SDL2_mixer directory" FORCE)
            else()
                set(SDL_MIXER_INCLUDE_DIR "${SDL2_MIXER_ROOT}/i686-w64-mingw32/include/SDL2")
                set(SDL_MIXER_LIB_DIR "${SDL2_MIXER_ROOT}/i686-w64-mingw32/lib")
                set(SDL2_MIXER_DYNAMIC_LIB_DIR "${SDL2_MIXER_ROOT}/i686-w64-mingw32/bin" CACHE PATH "SDL2_mixer directory" FORCE)
            endif()
            set(LIB_PATH "${SDL_MIXER_LIB_DIR}/libSDL2_mixer.a")
            set(DYNAMIC_LIB_PATH "${SDL_MIXER_LIB_DIR}/SDL2_mixer.dll")

            add_library(SDL2_mixer INTERFACE)
            target_include_directories(SDL2_mixer INTERFACE ${SDL_MIXER_INCLUDE_DIR})
            target_link_directories(SDL2_mixer INTERFACE ${SDL_MIXER_LIB_DIR})
            target_link_libraries(SDL2_mixer INTERFACE "-lSDL2_mixer")
        else()
            message(FATAL_ERROR "your compiler don't support, please use MSVC, Clang++ or MinGW")
        endif()

        mark_as_advanced(SDL2_MIXER_DYNAMIC_LIB_DIR)
    else()  # Linux, MacOSX
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2_mixer SDL2_mixer REQUIRED IMPORTED_TARGET)
        add_library(SDL2_mixer ALIAS PkgConfig::SDL2_mixer)
    endif()
endif()