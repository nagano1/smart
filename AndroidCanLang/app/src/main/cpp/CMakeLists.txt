# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)



# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# Sets lib_src_DIR to the path of the target CMake project.
set( lib_src_dir ../../../../../src )
set( tests_src_dir ../../../../../tests/google_tests )

# Sets lib_build_DIR to the path of the desired output directory.
# set( lib_build_DIR ../../../../../src/outputs )
# file(MAKE_DIRECTORY ${lib_build_DIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#set(CMAKE_SHARED_LINKER_FLAGS "-Wall -v -Wl,--no-warn-shared-textrel")
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11")

#add_subdirectory(cpu_features)

include_directories(${lib_src_dir})

add_definitions("-DCMAKE_TEST=1")

file(GLOB code_nodes_files "${lib_src_dir}/codenodes/*.cpp")
file(GLOB script_runtime_files "${lib_src_dir}/script_runtime/*.cpp")

file(GLOB test_files "${tests_src_dir}/*.cpp")




add_library( # Sets the name of the library.
        native-lib


        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ./native-lib.cpp

        ${lib_src_dir}/thread_pool.cpp

        ${lib_src_dir}/LSP_server/LSPMain.cpp
        ${lib_src_dir}/LSP_server/LSPLocalServer.cpp
        ${code_nodes_files}
        ${script_runtime_files}

        ${tests_src_dir}/../testlib_impl.cpp
        ${test_files}

        )



#add_compile_options("$<$<CONFIG:DEBUG>:-Ofast>")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Ofast")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
#        cpu_features
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})


#set(USE_HTTPS "backend")
#add_subdirectory(libgit2)
