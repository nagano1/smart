#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <sstream>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>

#ifdef __x86_64__
// do x64 stuff
#elif __i386__
#elif __aarch64__
#elif __arm__
// do arm stuff
#endif



/*
Linux and Linux - derived           __linux__
Android                           __ANDROID__(implies __linux__)
Linux(non - Android)               __linux__ && !__ANDROID__
Darwin(Mac OS X and iOS)         __APPLE__
Akaros(http://akaros.org)        __ros__
Windows                           _WIN32
Windows 64 bit                    _WIN64(implies _WIN32)
NaCL                              __native_client__
AsmJS                             __asmjs__
Fuschia                           __Fuchsia__

Visual Studio       _MSC_VER
gcc                 __GNUC__
clang               __clang__
emscripten          __EMSCRIPTEN__(for asm.js and webassembly)
MinGW 32            __MINGW32__
MinGW - w64 32bit     __MINGW32__
MinGW - w64 64bit     __MINGW64__
*/

enum OS_TYPE {
    Windows,
    Linux,
    UNIX,
    Mac,
    iOS,
    Android
};


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define OF_WINDOWS 1
#include <windows.h>
#include <winnt.h>
constexpr OS_TYPE os_type = OS_TYPE::Windows;
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define __IOS__ 1
constexpr OS_TYPE os_type = OS_TYPE::iOS;
// iOS Simulator
#elif TARGET_OS_IPHONE
#define __IOS__ 1
constexpr OS_TYPE os_type = OS_TYPE::iOS;
// iOS device
#elif TARGET_OS_MAC
constexpr OS_TYPE os_type = OS_TYPE::Mac;
// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif
#elif __linux__
constexpr OS_TYPE os_type = OS_TYPE::Linux;
// linux
#elif __unix__ // all unices not caught above
constexpr OS_TYPE os_type = OS_TYPE::UNIX;
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif




//#ifndef  __ANDROID__
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define __MYWIN__
#include <emmintrin.h>
#endif

#ifdef __MYWIN__
#define do_pause_or_nothing() _mm_pause();
#else
#define do_pause_or_nothing() ;
#endif


#ifdef NO_SPEED_TEST
constexpr bool speed_test = false;
#else
constexpr bool speed_test = false;// true
#endif

#ifdef CMAKE_TEST
constexpr bool managed_cmake_test = true;
#else
constexpr bool managed_cmake_test = true; // false; // or visual studio embedded tests
#endif

#ifdef __ANDROID__

constexpr bool ARM = true;
#include "testlib_impl.hpp"
#include "thread_pool.hpp"

#elif defined(__IOS__)
constexpr bool ARM = true;
#include "testlib_impl.hpp"
#include "../src/thread_pool.hpp"
#else

constexpr bool ARM = false;

#include "gtest/gtest.h"
#include "thread_pool.hpp"
#endif




#ifdef __i386__
#ifdef __ANDROID__
#define __EMULATOR__ 1
#endif
#endif

#ifdef __EMULATOR__
constexpr bool EMULATOR = true;
#else
constexpr bool EMULATOR = false;
#endif



#ifndef ENDTEST
#define ENDTEST //[ENDTEST]
#endif



//#define CGTEST(cname, ctestname) TEST(cname, ctestname)
//#define PREPARE_OSTREAM std::ostringstream outputstream
#ifdef __ANDROID__
#include <unistd.h>


#define PREPARE_OSTREAM \
//Foo foo{};

#define GLOG smartlang::Log{}

#else
static std::ostream &GLOG = std::cout;

#endif


/*
 * () {
	static std::mutex mtx;
	return mtx;
}*/

