cmake_minimum_required(VERSION 3.20)
project(mpk_mix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

option(MPKMIX_BUILD_TESTING "Build test targets" ON)
option(MPKMIX_ENABLE_TZ "Enable timezone-aware functions (requires C++20 or date lib)" ON)

if(MPKMIX_BUILD_TESTING)
    enable_testing()
endif()

# Expose the deps helper to parent scope via a cache variable so sibling
# add_subdirectory() calls made after this one can include and use it.
include(cmake/mpkmix_deps.cmake)
set(MPKMIX_DEPS_CMAKE "${CMAKE_CURRENT_LIST_DIR}/cmake/mpkmix_deps.cmake"
    CACHE INTERNAL "Path to mpkmix_find_or_add helper")

# Phase 1 target: header-only, no external deps.
# Later phases (value, log, serial) will be added as compiled targets here.
add_library(mpk_mix_headers INTERFACE)
add_library(mpk_mix::headers ALIAS mpk_mix_headers)
target_include_directories(mpk_mix_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(mpk_mix_headers INTERFACE cxx_std_20)

add_subdirectory(3p)

target_link_libraries(mpk_mix_headers INTERFACE magic_enum::magic_enum)

if(NOT MPKMIX_COMPILER_HAS_STD_FORMAT)
    target_compile_definitions(mpk_mix_headers INTERFACE MPKMIX_USE_FMT_POLYFILL)
    target_link_libraries(mpk_mix_headers INTERFACE fmt::fmt-header-only)
endif()

if(NOT MPKMIX_ENABLE_TZ)
    target_compile_definitions(mpk_mix_headers INTERFACE MPKMIX_NO_TZ)
endif()

# Phase 2 target: compiled value/type/value_path system.
add_library(mpk_mix_value STATIC
    src/value/value_path.cpp
    src/value/type.cpp
    src/value/value.cpp
    src/value/parse_simple_value.cpp)
add_library(mpk_mix::value ALIAS mpk_mix_value)
target_link_libraries(mpk_mix_value
    PUBLIC
        mpk_mix::headers
        magic_enum::magic_enum)

# Phase 3 target: logging abstraction — header-only INTERFACE.
# Consuming projects link quill::quill themselves when QUILL engine is selected.
add_library(mpk_mix_log INTERFACE)
add_library(mpk_mix::log ALIAS mpk_mix_log)
target_link_libraries(mpk_mix_log INTERFACE mpk_mix::headers)

# Phase 4 target: YAML/JSON serialization.
add_library(mpk_mix_serial STATIC
    src/util/time_point.cpp
    src/serial/path.cpp
    src/serial/parse_simple_value.cpp
    src/serial/yaml/parse.cpp
    src/serial/yaml/parse_value.cpp
    src/serial/yaml/to_yaml.cpp
    src/serial/json/parse.cpp
    src/serial/json/from_file.cpp)
add_library(mpk_mix::serial ALIAS mpk_mix_serial)
target_link_libraries(mpk_mix_serial
    PUBLIC
        mpk_mix::headers
        mpk_mix::value
        magic_enum::magic_enum
        yaml-cpp::yaml-cpp
        nlohmann_json::nlohmann_json)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(mpk_mix_serial PRIVATE mpk_mix_date_tz)
endif()

# Phase 5 target: POSIX wrap_c — compiled STATIC, Linux-only.
add_library(mpk_mix_wrap_c STATIC
    src/wrap_c/check_c_call.cpp
    src/wrap_c/fd.cpp
    src/wrap_c/io.cpp
    src/wrap_c/host_addr.cpp
    src/wrap_c/get_ipv4_addr_of.cpp
    src/wrap_c/sock_addr.cpp
    src/wrap_c/socket.cpp)
add_library(mpk_mix::wrap_c ALIAS mpk_mix_wrap_c)
target_link_libraries(mpk_mix_wrap_c
    PUBLIC
        mpk_mix::log
        mpk_mix::headers)
# Default log engine for standalone build (no quill available here).
# Consuming projects (ildfar, pytrunk) compile sources themselves with their
# own MPKMIX_LOG_ENGINE setting propagated from their respective log configs.
target_compile_definitions(mpk_mix_wrap_c PRIVATE
    MPKMIX_LOG_ENGINE=MPKMIX_LOG_NONE)

# Phase 6 target: compiled util — scratch_dir, command_line_args, signal_handler.
add_library(mpk_mix_util STATIC
    src/util/scratch_dir.cpp
    src/util/command_line_args.cpp
    src/util/signal_handler.cpp
    src/util/stacktrace.cpp
    src/util/install_crash_handlers.cpp)
add_library(mpk_mix::util ALIAS mpk_mix_util)
target_link_libraries(mpk_mix_util
    PUBLIC
        mpk_mix::log
        mpk_mix::serial)
target_compile_definitions(mpk_mix_util PRIVATE
    MPKMIX_LOG_ENGINE=MPKMIX_LOG_NONE)

# Stack-trace support (C++23 <stacktrace>). Off by default for cross builds
# (GCC 11 musl toolchains have no <stacktrace>), on otherwise — but probed,
# so a host without the right libstdc++ also gets it auto-disabled.
if(CMAKE_CROSSCOMPILING)
    set(_mpkmix_stacktrace_default OFF)
else()
    set(_mpkmix_stacktrace_default ON)
endif()
option(MPKMIX_STACKTRACE
    "Capture stack traces at exception throw, terminate, and fatal signals"
    ${_mpkmix_stacktrace_default})

if(MPKMIX_STACKTRACE)
    include(CheckCXXSourceCompiles)
    set(_mpkmix_st_save_flags "${CMAKE_REQUIRED_FLAGS}")
    set(_mpkmix_st_save_libs  "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_FLAGS "-std=c++23")

    set(_mpkmix_st_lib "")
    foreach(_cand stdc++exp stdc++_libbacktrace)
        set(CMAKE_REQUIRED_LIBRARIES "${_cand};${CMAKE_DL_LIBS}")
        check_cxx_source_compiles(
            "#include <stacktrace>\nint main(){auto s=std::stacktrace::current();return (int)s.size();}"
            MPKMIX_STACKTRACE_HAS_${_cand})
        if(MPKMIX_STACKTRACE_HAS_${_cand})
            set(_mpkmix_st_lib "${_cand}")
            break()
        endif()
    endforeach()

    set(CMAKE_REQUIRED_FLAGS     "${_mpkmix_st_save_flags}")
    set(CMAKE_REQUIRED_LIBRARIES "${_mpkmix_st_save_libs}")

    if(_mpkmix_st_lib STREQUAL "")
        message(WARNING
            "MPKMIX_STACKTRACE requested but <stacktrace> is unusable with this "
            "toolchain; disabling.")
        set(MPKMIX_STACKTRACE OFF CACHE BOOL "" FORCE)
    else()
        message(STATUS "mpk_mix: stacktrace enabled (linking against ${_mpkmix_st_lib})")
        # The two stacktrace TUs need C++23.
        set_source_files_properties(
            src/util/stacktrace.cpp
            src/util/install_crash_handlers.cpp
            PROPERTIES COMPILE_OPTIONS "-std=c++23")
        target_compile_definitions(mpk_mix_util PUBLIC MPKMIX_STACKTRACE)
        target_link_libraries(mpk_mix_util PUBLIC ${_mpkmix_st_lib} ${CMAKE_DL_LIBS})
    endif()
endif()

if(MPKMIX_BUILD_TESTING)
    add_subdirectory(test)
endif()
