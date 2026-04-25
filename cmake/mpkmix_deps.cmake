# mpkmix_find_or_add(TARGET <cmake-target> PACKAGE <package-name> SUBDIR <path>)
#
# Guarded dependency inclusion — handles three cases in order:
#   1. TARGET already exists in scope (parent project added it first) → no-op.
#   2. find_package(PACKAGE) succeeds (Conan toolchain or system install) → use it.
#   3. Neither → add_subdirectory(SUBDIR) (bundled submodule or migration-lab path).
#
# When case 2 applies, the IMPORTED target is promoted to GLOBAL scope so that
# it is visible from parent directories (e.g. the top-level CMakeLists.txt that
# links against the target after this function returns).
#
# SUBDIR may be an absolute path (e.g. a migration-lab sibling) or a path
# relative to CMAKE_CURRENT_SOURCE_DIR. Absolute paths get an auto-generated
# binary directory so CMake does not reject out-of-tree sources.
function(mpkmix_find_or_add)
    cmake_parse_arguments(ARG "" "TARGET;PACKAGE;SUBDIR" "" ${ARGN})
    if(NOT TARGET ${ARG_TARGET})
        find_package(${ARG_PACKAGE} QUIET CONFIG)
        if(${ARG_PACKAGE}_FOUND)
            # Promote the IMPORTED target to global scope.
            # find_package() inside a CMake function creates non-global
            # IMPORTED targets; without promotion they are invisible outside
            # this directory scope (i.e. in the top-level CMakeLists.txt).
            if(TARGET ${ARG_TARGET})
                set_target_properties(${ARG_TARGET} PROPERTIES IMPORTED_GLOBAL TRUE)
            endif()
        else()
            if(IS_ABSOLUTE "${ARG_SUBDIR}")
                add_subdirectory(
                    "${ARG_SUBDIR}"
                    "${CMAKE_CURRENT_BINARY_DIR}/_deps/${ARG_PACKAGE}")
            else()
                add_subdirectory("${ARG_SUBDIR}")
            endif()
        endif()
    endif()
endfunction()
