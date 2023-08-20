function(FetchSDL_MSVC)
    include(FetchContent)
    
    message(STATUS "fetching SDL2 ...")
    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.24.2/SDL2-devel-2.24.2-VC.zip
    )

    message(STATUS "fetching SDL2_image ...")
    FetchContent_Declare(
        SDL2_image
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.2/SDL2_image-devel-2.6.2-VC.zip
    )
    
    message(STATUS "fetching SDL2_ttf ...")
    FetchContent_Declare(
        SDL2_ttf
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-devel-2.20.1-VC.zip
    )
    
    message(STATUS "fetching SDL2_mixer ...")
    FetchContent_Declare(
        SDL2_mixer
        URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.2/SDL2_mixer-devel-2.6.2-VC.zip
    )

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_ttf SDL2_mixer)

    set(SDL2_ROOT ${sdl2_SOURCE_DIR} CACHE PATH "SDL2 root directory")
    set(SDL2_MIXER_ROOT ${sdl2_mixer_SOURCE_DIR} CACHE PATH "SDL2_mixer root directory")
    set(SDL2_TTF_ROOT ${sdl2_ttf_SOURCE_DIR} CACHE PATH "SDL2_ttf root directory")
    set(SDL2_IMAGE_ROOT ${sdl2_image_SOURCE_DIR} CACHE PATH "SDL2_image root directory")
endfunction(FetchSDL_MSVC)

function(FetchSDL_MINGW)
    include(FetchContent)
    
    message(STATUS "fetching SDL2 ...")
    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.2/SDL2-devel-2.26.2-mingw.zip
    )

    message(STATUS "fetching SDL2_image ...")
    FetchContent_Declare(
        SDL2_image
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.2/SDL2_image-devel-2.6.2-mingw.zip
    )
    
    message(STATUS "fetching SDL2_ttf ...")
    FetchContent_Declare(
        SDL2_ttf
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-devel-2.20.1-mingw.zip
    )
    
    message(STATUS "fetching SDL2_mixer ...")
    FetchContent_Declare(
        SDL2_mixer
        URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.2/SDL2_mixer-devel-2.6.2-mingw.zip
    )

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_ttf SDL2_mixer)

    set(SDL2_ROOT ${sdl2_SOURCE_DIR} CACHE PATH "SDL2 root directory")
    set(SDL2_MIXER_ROOT ${sdl2_mixer_SOURCE_DIR} CACHE PATH "SDL2_mixer root directory")
    set(SDL2_TTF_ROOT ${sdl2_ttf_SOURCE_DIR} CACHE PATH "SDL2_ttf root directory")
    set(SDL2_IMAGE_ROOT ${sdl2_image_SOURCE_DIR} CACHE PATH "SDL2_image root directory")
endfunction(FetchSDL_MINGW)

function(FetchSDL)
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR
        (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
         CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")) # use MSVC
        FetchSDL_MSVC()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
            (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
             CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU")) # use MINGW
        FetchSDL_MINGW()
    endif()
endfunction()