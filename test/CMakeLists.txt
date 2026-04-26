cmake_minimum_required(VERSION 3.20)
project(mpk_mix_test LANGUAGES CXX)

add_executable(mpk_mix_test
    test_macro.cpp
    test_meta.cpp
    test_struct_type.cpp
    test_strong.cpp
    test_func_ref.cpp
    test_util.cpp
    test_types.cpp)

target_link_libraries(mpk_mix_test
    PRIVATE
        mpk_mix::headers
        GTest::gtest
        GTest::gtest_main)

# GCC 14 false-positive: inlining vector copy into Checkpoint::sync() at -O2
# triggers -Warray-bounds / -Wstringop-overflow despite the code being correct.
# -Wno-error= is needed when a parent project appends -Werror to CMAKE_CXX_FLAGS.
target_compile_options(mpk_mix_test PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-array-bounds -Wno-error=array-bounds -Wno-stringop-overflow -Wno-error=stringop-overflow>)

add_executable(mpk_mix_serial_test
    test_serial.cpp
    test_yaml_parse_value.cpp)

target_link_libraries(mpk_mix_serial_test
    PRIVATE
        mpk_mix::serial
        GTest::gtest
        GTest::gtest_main)

add_executable(mpk_mix_value_test
    test_enum_flags.cpp
    test_value.cpp)

target_link_libraries(mpk_mix_value_test
    PRIVATE
        mpk_mix::value
        GTest::gtest
        GTest::gtest_main)

add_executable(mpk_mix_wrap_c_test
    test_wrap_c.cpp)

target_link_libraries(mpk_mix_wrap_c_test
    PRIVATE
        mpk_mix::wrap_c
        GTest::gtest
        GTest::gtest_main)

target_compile_definitions(mpk_mix_wrap_c_test PRIVATE
    MPKMIX_LOG_ENGINE=MPKMIX_LOG_NONE)

add_executable(mpk_mix_util_test
    test_util_compiled.cpp)

target_link_libraries(mpk_mix_util_test
    PRIVATE
        mpk_mix::util
        GTest::gtest
        GTest::gtest_main)

target_compile_definitions(mpk_mix_util_test PRIVATE
    MPKMIX_LOG_ENGINE=MPKMIX_LOG_NONE)

# GCC 14 false-positive: inlining vector copy into Checkpoint::sync() at -O2
# triggers -Warray-bounds / -Wstringop-overflow despite the code being correct.
# -Wno-error= is needed when a parent project appends -Werror to CMAKE_CXX_FLAGS.
target_compile_options(mpk_mix_util_test PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-array-bounds -Wno-error=array-bounds -Wno-stringop-overflow -Wno-error=stringop-overflow>)

include(GoogleTest)
gtest_discover_tests(mpk_mix_test)
gtest_discover_tests(mpk_mix_serial_test)
gtest_discover_tests(mpk_mix_value_test)
gtest_discover_tests(mpk_mix_wrap_c_test)
gtest_discover_tests(mpk_mix_util_test)
