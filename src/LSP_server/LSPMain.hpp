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

struct LSPManager {
    static void LSP_main();
    
    void nextRequest(char *str, st_textlen length);
};


//struct LSPMessage {
//    struct Param {
//        char name[64];
//        char value[64];
//        struct Param *next;
//    };
//    int contentLength;
//    char method[128];
//    struct Param *firstParam;
//
//        /*
//
//    Content-Length: 200
//
//    JSON{"jsonrpc":"2.0","method":"initialized","params":{}}
//     */
//
//
//};


struct LSPParser {
    //LSPMessage message;
    bool nextByte(utf8byte b);
};

