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
#if defined(__i386__) || defined(__x86_64__)
#include <emmintrin.h>
#endif

#ifdef __x86_64__    
	// do x64 stuff   
#elif __arm__    
	// do arm stuff
#endif