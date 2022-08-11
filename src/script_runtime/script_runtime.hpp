#pragma once

#include <stdlib.h>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>

#include <cstdint> // uint64_t, int_fast32_t
#include <ctime>

#include <string.h> // memcpy

#include "parse_util.hpp"
#include "common.hpp"

#include "code_nodes.hpp"



namespace smart {

    using ValueBase = struct _valueBase {
        int typeIndex;
        void *ptr;
        int size; // byte size
    };

    using IntValue = struct {
        void *ptr;
        int size; // byte size
    };


    void startScript(DocumentStruct* doc);
    
}