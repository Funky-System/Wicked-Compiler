set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

set(CMAKE_BUILD_TYPE Release)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
        gtest
        URL https://github.com/google/googletest/archive/master.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        # Disable install step
        INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )

# Create a libgmock target to be used as a dependency by test programs
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# Set libgmock properties
set_target_properties(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/googletest/include"
        "${source_dir}/googlemock/include")



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
    set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/libfunky-as.a"
            "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
    include_directories("${source_dir}/include")
endif()

if (NOT TARGET funky-vm)
    ExternalProject_Add(
            EXT_funky-vm
            GIT_REPOSITORY https://github.com/Funky-System/Funky-VM.git
            GIT_TAG master
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/funky-vm
            # Disable install step
            INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(EXT_funky-vm source_dir binary_dir)
    add_library(funky-vm IMPORTED STATIC GLOBAL)
    add_dependencies(funky-vm EXT_funky-vm)
    set_target_properties(funky-vm PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/libfunky-vm.a"
            "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
    include_directories("${source_dir}/include")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testwickedc ${SRCS} frontend.cpp frontend.h basic.spec.cpp functions.spec.cpp numerical_expressions.spec.cpp if.spec.cpp classes.spec.cpp strings.spec.cpp arrays.spec.cpp loops.spec.cpp enums.spec.cpp maps.spec.cpp comparisons.spec.cpp scopes.spec.cpp)

#add_dependencies(testwickedc funky-as)
TARGET_LINK_LIBRARIES(testwickedc libgtest libgmock wickedc funky-as funky-vm)

add_test(NAME testwickedc COMMAND testwickedc)

configure_file(testlib.fasm ${CMAKE_BINARY_DIR}/test/testlib.fasm COPYONLY)
