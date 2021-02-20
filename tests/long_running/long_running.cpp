#include "long_running.h"

#include <stdio.h>
#include <stdlib.h>
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

struct S {
    int a;
};

int main(int argc, char **argv) {

    srand((unsigned int)time(NULL));

for (int i = 0; i < 200*1000; i++)  {

    int max  = rand() % 10000;

    for (int j = 0; j < max; j++)  {
            MallocBuffer charBuffer3;
            charBuffer3.init();;
            int len  = rand() % 1000;

            int size = 355;
            auto *chars = charBuffer3.newChars<S>(sizeof(S)*len);
            chars->a = 5;

            auto *chars2 = charBuffer3.newChars<S>(sizeof(S));
            chars2->a = 2;

            charBuffer3.tryDelete(chars);

            charBuffer3.freeAll();


    }
}

/*
for (int i = 0; i < 200*1000; i++) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));


        auto *text = const_cast<char *>(u8R"(
{
        "aowowo" :    21249,
"jio fw" : null,
            "text" : "日本語"
            , "ijofw": [2134
                  	    ,
                            "test", true,
                        null,
                        {"君はどうなんだろう": [true]}
            ]

})");
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));

        if (document->context->syntaxErrorInfo.errorCode ==  21390) {

        }
        Alloc::deleteDocument(document);

    }
*/

	return 0;
}
