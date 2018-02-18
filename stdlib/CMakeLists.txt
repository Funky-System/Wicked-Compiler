cmake_minimum_required(VERSION 3.5)
project(stdlib)

set(CMAKE_C_STANDARD 11)

ExternalProject_Add(
        STDLIB_funky-as
        GIT_REPOSITORY https://github.com/Funky-System/Funky-Assembler.git
        GIT_TAG master
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/funky-as
        # Disable install step
        INSTALL_COMMAND ""
)
set(ASSEMBLER ${CMAKE_CURRENT_BINARY_DIR}/funky-as/src/STDLIB_funky-as-build/funky-as)

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/stdlib/array.funk
        COMMAND ls
        DEPENDS wickedc-bin
)
add_custom_target(stdlib
        ../wickedc ${CMAKE_CURRENT_SOURCE_DIR}/array.wckd --assembler ${ASSEMBLER} --output=array.funk --keep-asm
        DEPENDS wickedc-bin STDLIB_funky-as
        )