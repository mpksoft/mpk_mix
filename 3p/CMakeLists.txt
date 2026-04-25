cmake_minimum_required(VERSION 3.20)

# Resolve real path before computing relative paths — CMAKE_CURRENT_LIST_DIR
# may point through a symlink (e.g. 3p/mpk_mix -> ../../mpk_mix) and relative
# navigation from the symlinked location would land in the wrong place.
get_filename_component(_mpkmix_3p_real_dir "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
include(${_mpkmix_3p_real_dir}/../cmake/mpkmix_deps.cmake)

mpkmix_find_or_add(
    TARGET  magic_enum::magic_enum
    PACKAGE magic_enum
    SUBDIR  ${_mpkmix_3p_real_dir}/magic_enum)

# magic_enum's cmake target exposes include/magic_enum/ (bare <magic_enum.hpp>
# style). We use <magic_enum/magic_enum.hpp> style, so also expose include/.
# Only needed when using the bundled submodule: add_subdirectory() creates a
# real (non-IMPORTED) target named "magic_enum"; find_package() (e.g. Conan)
# creates only the namespaced "magic_enum::magic_enum" IMPORTED target which
# already carries the correct include path and cannot be modified.
if(TARGET magic_enum)
    target_include_directories(magic_enum INTERFACE
        $<BUILD_INTERFACE:${_mpkmix_3p_real_dir}/magic_enum/include>)
endif()

mpkmix_find_or_add(
    TARGET  yaml-cpp::yaml-cpp
    PACKAGE yaml-cpp
    SUBDIR  ${_mpkmix_3p_real_dir}/yaml-cpp)

mpkmix_find_or_add(
    TARGET  nlohmann_json::nlohmann_json
    PACKAGE nlohmann_json
    SUBDIR  ${_mpkmix_3p_real_dir}/nlohmann_json)

if(MPKMIX_BUILD_TESTING)
    mpkmix_find_or_add(
        TARGET  GTest::gtest
        PACKAGE GTest
        SUBDIR  ${_mpkmix_3p_real_dir}/googletest)
endif()

# date is a clang-only dependency: GCC uses std::chrono timezone support natively.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(BUILD_TZ_LIB ON CACHE BOOL "Build date timezone library" FORCE)
    set(USE_SYSTEM_TZ_DB ON CACHE BOOL "Use system timezone database" FORCE)
    mpkmix_find_or_add(
        TARGET  date::date
        PACKAGE date
        SUBDIR  ${_mpkmix_3p_real_dir}/date)
    # Normalise target name: the submodule splits headers (date::date, INTERFACE)
    # from the compiled tz library (date::date-tz); Conan folds both into
    # date::date. mpk_mix_date_tz always resolves to the compiled tz target.
    add_library(mpk_mix_date_tz INTERFACE)
    if(TARGET date::date-tz)
        target_link_libraries(mpk_mix_date_tz INTERFACE date::date-tz)
    else()
        target_link_libraries(mpk_mix_date_tz INTERFACE date::date)
    endif()
endif()
