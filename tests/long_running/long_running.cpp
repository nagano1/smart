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

#include "code_nodes.hpp"
#include "parse_util.hpp"

using namespace smart;

struct S
{
    int a;
};

int main(int argc, char **argv)
{
    // long running test
    srand((unsigned int)time(NULL));
    
    while (true) {
        for (int i = 0; i < 80; i++) {
            unsigned int max = 1 + rand() % 1000;

            MemBuffer *charBuffer3 = (MemBuffer *)malloc(sizeof(MemBuffer));
            charBuffer3->init();

            for (int j = 0; j < max; j++) {
                unsigned int len = 1 + rand() % 100;

                auto *chars = charBuffer3->newMem<S>(len);
                chars->a = 5;

                auto *chars2 = charBuffer3->newMem<S>(1);
                chars2->a = 2;

                charBuffer3->tryDelete(chars);
                charBuffer3->tryDelete(chars2);
            }
            charBuffer3->freeAll();
            free(charBuffer3);
        }



    //for (int i = 0; i < 1000*1000*1000; i++) {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));


        {
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

                Alloc::deleteDocument(document);



        }

        {
                    auto *text = const_cast<char *>(u8R"(
class fjawioejap
{
    fn funcB()
    {
        let a = 893214
        let *str = "0jfoiwjoie"
        int ab = 123412
        "fjoiiw" // u889u98u



        $float f = 4503
        ?let *f = null
        $let g = true
        
        "abcdefg"
        3142
        true
        null
        false

        "jfiowjaoie"
        123490
        false
        null
        true
        /*
        ofijaweifaow 
        aiwjeofijaw jpawijfw4213
        jioawjeifoaw
        jfaiwejfiwoajoijw
        jofijeiowajiwa
        */
        // joiwfaejfo
        // joijweoifjaw oiaoiwjfoijwaioji

        let *abc = "joifwjoe01234"
        let f = 343214213
        int a = 3124
        2134
        4123

        afwfiiaeawe
        taijetjwap
        faijefjwap
        true
        "jofoajwieofawo"
    }
}




class A
{
    class AClass/**/
    {/**/
        // awef
        fn ffanc()
        {
            3142314
            false
            // jfoiaweoifaw
            
            func(true, "jfoiw", 1203)
            
            return/*true*/3241//面白すぎ
            return/**/21241
            false//tugi ga saigono kyokudesu
            return 1

        }


        fn a()
        {
            // afjiowe

        } // afweo
    } // joiwafjoefwa 
    //
    /* fwaei */
}


        )");
                auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
                DocumentUtils::parseText(document, text, strlen(text));
                char *treeText = DocumentUtils::getTextFromTree(document);

                free(treeText);
                Alloc::deleteDocument(document);
        }





    }

    return 0;
}
