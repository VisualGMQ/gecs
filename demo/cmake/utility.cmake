function(IsMSVCBackend RETURN_VALUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR
        (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
        set(${RETURN_VALUE} 1 PARENT_SCOPE)
    else()
        set(${RETURN_VALUE} 0 PARENT_SCOPE)
    endif()
endfunction()

function(IsMinGWBackend RETURN_VALUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU" AND WIN32)) # use MINGW
        set(${RETURN_VALUE} 1 PARENT_SCOPE)
    else()
        set(${RETURN_VALUE} 0 PARENT_SCOPE)
    endif()
endfunction()

function(IsGNUBackend RETURN_VALUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU" AND (UNIX OR APPLE))) # use GNU G++
        set(${RETURN_VALUE} 1 PARENT_SCOPE)
    else()
        set(${RETURN_VALUE} 0 PARENT_SCOPE)
    endif()
endfunction()

function(IsX64Compiler RETURN_VALUE)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(${RETURN_VALUE} 1 PARENT_SCOPE)
    else()
        set(${RETURN_VALUE} 0 PARENT_SCOPE)
    endif()
endfunction()