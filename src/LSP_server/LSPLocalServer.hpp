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

#include "common.hpp"

//#include <emmintrin.h>

struct LSPHttpServer{
    static void LSP_server();
    static void close();
    static void passText(char *text, int textLen);
};

