if (NOT TARGET SDL2)
    if (NOT SDL2_ROOT)
        set(SDL2_ROOT "" CACHE PATH "SDL2 root directory")
    endif()
    if (WIN32)  # Windows
        IsMSVCBackend(is_msvc_backend)
        IsMinGWBackend(is_mingw_backend)
        IsX64Compiler(is_x64_compiler)
        if (${is_msvc_backend}) # use MSVC
            if(${is_x64_compiler})
                set(SDL_LIB_DIR "${SDL2_ROOT}/lib/x64")
                set(SDL2_DYNAMIC_LIB_DIR "${SDL2_ROOT}/lib/x64" CACHE PATH "SDL2.dll directory" FORCE)
            else()
                set(SDL_LIB_DIR "${SDL2_ROOT}/lib/x86")
                set(SDL2_DYNAMIC_LIB_DIR "${SDL2_ROOT}/lib/x86" CACHE PATH "SDL2.dll directory" FROCE)
            endif()
            set(LIB_PATH "${SDL_LIB_DIR}/SDL2.lib")
            set(DYNAMIC_LIB_PATH "${SDL2_DYNAMIC_LIB_DIR}/SDL2.dll")
            set(MAIN_LIB_PATH "${SDL_LIB_DIR}/SDL2main.lib")
            set(SDL_INCLUDE_DIR "${SDL2_ROOT}/include")

            mark_as_advanced(SDL2_DYNAMIC_LIB_DIR)
            add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)
            set_target_properties(
                SDL2::SDL2
                PROPERTIES
                    IMPORTED_LOCATION ${DYNAMIC_LIB_PATH}
                    IMPORTED_IMPLIB ${LIB_PATH}
                    INTERFACE_INCLUDE_DIRECTORIES ${SDL_INCLUDE_DIR}
            )
            add_library(SDL2::SDL2main SHARED IMPORTED GLOBAL)
            set_target_properties(
                SDL2::SDL2main
                PROPERTIES
                    IMPORTED_LOCATION ${DYNAMIC_LIB_PATH}
                    IMPORTED_IMPLIB ${MAIN_LIB_PATH}
                    INTERFACE_INCLUDE_DIRECTORIES ${SDL_INCLUDE_DIR}
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            )
            add_library(SDL2 INTERFACE IMPORTED GLOBAL)
            target_link_libraries(SDL2 INTERFACE SDL2::SDL2main SDL2::SDL2)
        elseif (${is_mingw_backend}) # use MINGW
            if(${is_x64_compiler})
                set(SDL_INCLUDE_DIR "${SDL2_ROOT}/x86_64-w64-mingw32/include/SDL2")
                set(SDL_LIB_DIR "${SDL2_ROOT}/x86_64-w64-mingw32/lib")
                set(SDL2_DYNAMIC_LIB_DIR "${SDL2_ROOT}/x86_64-w64-mingw32/bin" CACHE PATH "SDL2.dll directory" FORCE)
            else()
                set(SDL_INCLUDE_DIR "${SDL2_ROOT}/i686-w64-mingw32/include/SDL2")
                set(SDL_LIB_DIR "${SDL2_ROOT}/i686-w64-mingw32/lib")
                set(SDL2_DYNAMIC_LIB_DIR "${SDL2_ROOT}/i686-w64-mingw32/bin" CACHE PATH "SDL2.dll directory" FORCE)
            endif()

            mark_as_advanced(SDL2_DYNAMIC_LIB_DIR)

            add_library(SDL2 INTERFACE)
            target_link_directories(SDL2 INTERFACE ${SDL_LIB_DIR})
            target_link_libraries(SDL2 INTERFACE "-lmingw32 -lSDL2main -lSDL2 -mwindows")
            target_include_directories(SDL2 INTERFACE ${SDL_INCLUDE_DIR})
        else()
            message(FATAL_ERROR "your compiler don't support, please use MSVC, Clang++ or MinGW")
        endif()
    else()  # Linux, MacOSX
        find_package(SDL2 QUIET)
        if (SDL2_FOUND)
            add_library(SDL2 ALIAS SDL2::SDL2)
        else()
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(SDL2 sdl2 REQUIRED IMPORTED_TARGET)
            add_library(SDL2 ALIAS PkgConfig::SDL2)
        endif()
    endif()
endif()