cmake_minimum_required(VERSION 3.5)

project(wickedc)

set(CMAKE_C_STANDARD 11)

include(ExternalProject)

if(NOT DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release" FORCE)
endif()

message("Compiler identified as ${CMAKE_C_COMPILER_ID}")
if(COVERAGE)
        message("Coverage requested")
        if (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
                message("  -> enabled for Clang")
                set(CMAKE_C_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
                set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
                set(CMAKE_EXE_LINKER_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
        elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
                message("  -> enabled for GCC")
                set(CMAKE_C_FLAGS "--coverage")
                set(CMAKE_CXX_FLAGS "--coverage")
                set(CMAKE_EXE_LINKER_FLAGS "--coverage")
                include(CTestCoverageCollectGCOV)
                ctest_coverage_collect_gcov(TARBALL gcov.tar)
        endif()
endif()

IF(APPLE)
        set(CMAKE_THREAD_LIBS_INIT "-lpthread")
        set(CMAKE_HAVE_THREADS_LIBRARY 1)
        set(CMAKE_USE_WIN32_THREADS_INIT 0)
        set(CMAKE_USE_PTHREADS_INIT 1)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
ENDIF()

include_directories(include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(BASH_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(BASH_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
if(MSVC)
        execute_process(COMMAND wsl wslpath "${BASH_SOURCE_DIR}" OUTPUT_VARIABLE BASH_SOURCE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND wsl wslpath "${BASH_BINARY_DIR}" OUTPUT_VARIABLE BASH_BINARY_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c" "${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c" "${CMAKE_CURRENT_BINARY_DIR}/parser.gen.h"
        COMMAND bash "${BASH_SOURCE_DIR}/tools/generate_parser.sh" grammar "${BASH_SOURCE_DIR}/src/libwickedc/wicked.grammar" "${BASH_BINARY_DIR}/grammar.gen.c" "${BASH_BINARY_DIR}/parser.gen.c" "${BASH_BINARY_DIR}/parser.gen.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/libwickedc/wicked.grammar"
        VERBATIM
)

if (NOT TARGET funky-as)
        ExternalProject_Add(
                EXT_funky-as
                GIT_REPOSITORY https://github.com/Funky-System/Funky-Assembler.git
                GIT_TAG master
                PREFIX ${CMAKE_CURRENT_BINARY_DIR}/funky-as
                # Disable install step
                INSTALL_COMMAND ""
        )
        ExternalProject_Get_Property(EXT_funky-as source_dir binary_dir)
        add_library(funky-as IMPORTED STATIC GLOBAL)
        add_dependencies(funky-as EXT_funky-as)
        if (MSVC)
                set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/${CMAKE_BUILD_TYPE}/funky-as.lib"
                        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
        else()
                if (CMAKE_CONFIGURATION_TYPES)
                        set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/${CMAKE_BUILD_TYPE}/libfunky-as.a"
                                "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
                else()
                        set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/libfunky-as.a"
                                "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
                endif()
        endif()
        include_directories("${source_dir}/include")
endif()

add_library(wickedc src/libwickedc/mpc/mpc.c
        "${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c"
        "${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c"
        src/libwickedc/hashmap.c
        src/libwickedc/compiler.c
        src/libwickedc/generators/generator.c
        src/libwickedc/generators/generator_expressions.c
        src/libwickedc/generators/generator_symbols.c
        src/libwickedc/generators/generator_functions.c
        src/libwickedc/generators/generator_statements.c
        src/libwickedc/generators/generator_variables.c
        src/libwickedc/generators/generator_if.c
        src/libwickedc/generators/generator_while.c
        src/libwickedc/generators/generator_arrays.c
        src/libwickedc/generators/generator_class.c
        src/libwickedc/generators/generator_modules.c
        src/libwickedc/generators/generator_enum.c
        src/libwickedc/string_functions.c)
add_executable(wickedc-bin src/main.c src/version.h)
target_link_libraries(wickedc-bin wickedc funky-as)
set_target_properties(wickedc-bin PROPERTIES OUTPUT_NAME wickedc)

file(COPY include DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS wickedc-bin DESTINATION bin)

if (NOT DEFINED TESTING_ENABLED)
        set(TESTING_ENABLED true)
endif()

if (TESTING_ENABLED)
        enable_testing()
        add_subdirectory(test)
endif()

add_subdirectory(stdlib)

if(COVERAGE)
        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
                include(CTestCoverageCollectGCOV)
                ctest_coverage_collect_gcov(TARBALL gcov.tar)
        endif()
endif()