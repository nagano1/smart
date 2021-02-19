#include "long_running.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
#include <emmintrin.h>

#include "code_nodes.hpp"
#include "parse_util.hpp"

using namespace smart;

int main(int argc, char **argv) {


for (int i = 0; i < 10000; i++) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        char *text = const_cast<char *>(u8R"( {"jsonrpc":"2.0", "method" : "initialized
)");
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));

        if (document->context->syntaxErrorInfo.errorCode ==  21390) {

        }
    }


	return 0;
}
