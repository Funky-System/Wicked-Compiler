cmake_minimum_required(VERSION 3.5)
project(wickedc)

set(CMAKE_C_STANDARD 11)

include(ExternalProject)

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
target_link_libraries(wickedc-bin wickedc)
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