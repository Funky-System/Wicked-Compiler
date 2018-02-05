cmake_minimum_required(VERSION 3.5)
project(wickedc)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.h
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/generate_parser.sh grammar ${CMAKE_CURRENT_SOURCE_DIR}/wicked.grammar ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.h
        DEPENDS wicked.grammar
)

add_library(wickedc mpc/mpc.c ${CMAKE_CURRENT_BINARY_DIR}/grammar.gen.c ${CMAKE_CURRENT_BINARY_DIR}/parser.gen.c generator.c generator.h hashmap.c generator_expressions.c generator_symbols.c generator_functions.c generator_statements.c generator_variables.c generator_if.c generator_while.c generator_arrays.c compiler.c compiler.h generator_class.c)
add_executable(wickedc-bin main.c)
target_link_libraries(wickedc-bin wickedc)
set_target_properties(wickedc-bin PROPERTIES OUTPUT_NAME wickedc)
