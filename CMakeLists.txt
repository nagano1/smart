cmake_minimum_required(VERSION 3.12)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11) # 11, 14, C++14...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

# /c /Zi /nologo /diagnostics:classic /sdl /O2 /Oi /GL /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /arch:SSE2 /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline


if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /source-charset:utf-8")
    # -DCMAKE_GENERATOR_PLATFORM=x86 . 
else()
    set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -Wall")
endif()


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")




project(main)

include_directories(src)


#enable_testing()
#add_test(NAME main COMMAND main)


add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tests/long_running)
add_subdirectory(lsp_vscode_server)
add_subdirectory(lib/googletest)

if(DOOR32 EQUAL 1)
    set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

