cmake_minimum_required(VERSION 3.5)
project(wickedc)

set(CMAKE_C_STANDARD 11)

include(ExternalProject)

include_directories(include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.h
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_parser.sh grammar ${CMAKE_CURRENT_SOURCE_DIR}/src/libwickedc/wicked.grammar ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/libwickedc/wicked.grammar
)

add_library(wickedc src/libwickedc/mpc/mpc.c
        ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c
        ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c
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

enable_testing()
add_subdirectory(test)

add_subdirectory(stdlib)