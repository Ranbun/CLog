cmake_minimum_required(VERSION 3.15)

set(target_name CmarkTest)
project(${target_name})

add_executable(${target_name} test.cpp)

IF(WITH_TEST)
    add_test(NAME ${target_name}_test COMMAND ${target_name})
    set_property(TEST ${target_name}_test PROPERTY WILL_PAIL true)
ENDIF(WITH_TEST)

