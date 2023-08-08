macro(AddTest name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE gecs)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
endmacro(AddTest)

AddTest(sparse_set)
