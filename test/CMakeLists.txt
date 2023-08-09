macro(AddTest name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE gecs)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/test)
endmacro(AddTest)

add_subdirectory(entity)
add_subdirectory(signal)