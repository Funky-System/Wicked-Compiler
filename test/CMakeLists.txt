set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    message("CMAKE_BUILD_TYPE not set, autosetting to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release" FORCE)
endif()

# Enable ExternalProject CMake module
include(ExternalProject)
include(FetchContent)

# Download and install GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


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
        if (GENERATOR_IS_MULTI_CONFIG)
            set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/${CMAKE_BUILD_TYPE}/libfunky-as.a"
                    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
        else()
            set_target_properties(funky-as PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/libfunky-as.a"
                    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
        endif()
    endif()
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
    if (MSVC)
        set_target_properties(funky-vm PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/${CMAKE_BUILD_TYPE}/funky-vm.lib"
            "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
    else()
        if (GENERATOR_IS_MULTI_CONFIG)
            set_target_properties(funky-vm PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/${CMAKE_BUILD_TYPE}/libfunky-vm.a"
                    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
        else()
            set_target_properties(funky-vm PROPERTIES "IMPORTED_LOCATION" "${binary_dir}/libfunky-vm.a"
                    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
        endif()
    endif()
    include_directories("${source_dir}/include")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testwickedc ${SRCS} frontend.cpp frontend.h basic.spec.cpp functions.spec.cpp numerical_expressions.spec.cpp if.spec.cpp classes.spec.cpp strings.spec.cpp arrays.spec.cpp loops.spec.cpp enums.spec.cpp maps.spec.cpp comparisons.spec.cpp scopes.spec.cpp)

#add_dependencies(testwickedc funky-as)
TARGET_LINK_LIBRARIES(testwickedc gtest_main wickedc funky-as funky-vm)

include(GoogleTest)
gtest_discover_tests(testwickedc DISCOVERY_MODE PRE_TEST)

if (MSVC)
    configure_file(testlib.fasm ${CMAKE_BINARY_DIR}/test/${CMAKE_BUILD_TYPE}/testlib.fasm COPYONLY)
else()
    if (GENERATOR_IS_MULTI_CONFIG)
        configure_file(testlib.fasm ${CMAKE_BINARY_DIR}/test/${CMAKE_BUILD_TYPE}/testlib.fasm COPYONLY)
    else()
        configure_file(testlib.fasm ${CMAKE_BINARY_DIR}/test/testlib.fasm COPYONLY)
    endif()
endif()