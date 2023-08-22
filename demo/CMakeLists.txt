aux_source_directory(src DEMO_SRC)

include(cmake/utility.cmake)
include(cmake/copydll.cmake)
include(cmake/FindSDL2.cmake)

add_executable(demo ${DEMO_SRC})
target_link_libraries(demo PRIVATE gecs SDL2)
target_include_directories(demo PRIVATE ./include)
target_compile_features(demo PRIVATE cxx_std_17)
CopyDLL(demo)