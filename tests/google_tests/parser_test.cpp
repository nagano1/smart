﻿#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "../test_common.h"

using namespace smart;




static void testJson(const char* codeText);

TEST(ParserTest_, JsonParseTest) {

    {
        // preserve spaces and line-breaks
        char *text = const_cast<char *>(u8R"(
{"jsonrpc":"2.0","id":0,"method":"initialize","params":{"processId":28196,"rootPath":null,"rootUri":null,"capabilities":{"workspace":{"applyEdit":true,"workspaceEdit":{"documentChanges":true},"didChangeConfiguration":{"dynamicRegistration":true},"didChangeWatchedFiles":{"dynamicRegistration":true},"symbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"executeCommand":{"dynamicRegistration":true},"configuration":true,"workspaceFolders":true},"textDocument":{"publishDiagnostics":{"relatedInformation":true},"synchronization":{"dynamicRegistration":true,"willSave":true,"willSaveWaitUntil":true,"didSave":true},"completion":{"dynamicRegistration":true,"contextSupport":true,"completionItem":{"snippetSupport":true,"commitCharactersSupport":true,"documentationFormat":["markdown","plaintext"],"deprecatedSupport":true},"completionItemKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}},"hover":{"dynamicRegistration":true,"contentFormat":["markdown","plaintext"]},"signatureHelp":{"dynamicRegistration":true,"signatureInformation":{"documentationFormat":["markdown","plaintext"]}},"definition":{"dynamicRegistration":true},"references":{"dynamicRegistration":true},"documentHighlight":{"dynamicRegistration":true},"documentSymbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"codeAction":{"dynamicRegistration":true},"codeLens":{"dynamicRegistration":true},"formatting":{"dynamicRegistration":true},"rangeFormatting":{"dynamicRegistration":true},"onTypeFormatting":{"dynamicRegistration":true},"rename":{"dynamicRegistration":true},"documentLink":{"dynamicRegistration":true},"typeDefinition":{"dynamicRegistration":true},"implementation":{"dynamicRegistration":true},"colorProvider":{"dynamicRegistration":true}}},"trace":"off","workspaceFolders":null}}
)");
        testJson(text);
    }

    {
        char *text = const_cast<char *>(u8R"( {"jsonrpc":"2.0", "method" : "initialized
)");
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));


        EXPECT_EQ(document->context->syntaxErrorInfo.hasError, true);
        EXPECT_EQ(static_cast<int>(document->context->syntaxErrorInfo.errorCode), static_cast<int>(ErrorCode::missing_closing_quote));
        EXPECT_EQ(std::string{ document->context->syntaxErrorInfo.reason }, std::string{ "missing closing quote" });
    }



    {
        char *text = const_cast<char *>(u8R"( {"jsonrpc":"2.0", "method" : "\"initialized\""})");
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        auto *rootJson = Cast::downcast<JsonObjectStruct*>(document->firstRootNode);
        EXPECT_NE(rootJson, nullptr);

        DocumentUtils::generateHashTables(document);
        auto *item = rootJson->hashMap->get2("method");

        EXPECT_NE(item, nullptr);
        EXPECT_EQ(item->vtable, VTables::StringLiteralVTable);

        auto *strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
        EXPECT_EQ( strNode->strLength, 13);
        EXPECT_EQ( strNode->textLength, 17);
        EXPECT_EQ(std::string{ strNode->str }, std::string{ "\"initialized\"" });
        //EXPECT_EQ(std::string{ strNode->text }, std::string{ "\\\"initialized\\\"" });
    }









    /*
    {
        char *text = const_cast<char *>(u8R"({"jsonrpc":"2.0", "method" : "textDocument/didOpen", "params" : {"textDocument":{"uri":"file:///c%3A/Users/wikihow/Desktop/AAA.txt", "languageId" : "plaintext", "version" : 1, "text" : "AAA\r\n\r\n\r\n\r\nBBB\r\nCCC\r\nAAA\r\nBBB"}}})");
        testJson(text);
    }
    */


    // follow indent rule
    {
        auto text = const_cast<char *>(u8R"(
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
        testJson(text);
    }
    {

        auto text = const_cast<char *>(u8R"({"aowfowo" : "😀😁😂ネコの顔文字と💘❤💓", "jiofw": false})");
        testJson(text);
    }


    testJson("{}");    // empty json object
    testJson("[]");    // empty json array
    testJson("[298341,12432134, true, false, \"fwo\", null]");

    testJson(u8R"({"empty_array" : [
            421,true, "ijofwe", null,false]})");

    {
        // nested json object
        auto text = const_cast<char *>(u8R"({"jfoiw": { "fjioew"  :   { "jfiow": true } 

        } 

            })");
        testJson(text);
    }

    {
        // last comma is allowed
        auto text = const_cast<char *>(u8R"({"jfoiw": true

    , 
}
)");
        testJson(text);
    }
}

static void testJson(const char* codeText) {
    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, codeText, strlen(codeText));
    //char *typeText = DocumentUtils::getTypeTextFromTree(document);
    //    if (typeText != nullptr) {
            //EXPECT_EQ(std::string{ typeText }, std::string{ "fjow" });
        //}

    char *treeText = DocumentUtils::getTextFromTree(document);
    DocumentUtils::generateHashTables(document);
    /*
    auto *jsonObject = DocumentUtils::generateHashTables(document);
    if (jsonObject) {
        auto *item = jsonObject->hashMap->get2("aowfowo");
        printf("item - %d", item);
    }
    */

    //if (treeText != nullptr) {
        //Allocator::deleteDocument(document);
        // EXPECT_EQ(std::string(treeText), "\n{b:18}\n");
    EXPECT_EQ(std::string{ treeText }, std::string{ codeText });
    //}

}


ENDTEST


/*
 test of iterating chars over utf8 text
 Point here is just whether you can distinguish ascii code from others even if it contains a wide char like emoji
*/
TEST(ParserTest_, char_iteration) {

    std::string wstr{ u8"auto * 😂日本語たち=10234;" };
    std::string wstr2{ "class TestClass{ }" };
    //int alen = wstr.length();
    auto chs = std::vector<char>{};

    //chs.push_back(0xEF);
    //chs.push_back(0xBB);
    //chs.push_back(0xBF);

    //std::cout << "\n[" << wstr.length() << "\n";

    char *text = const_cast<char *>(wstr.c_str());
    for (int i = 0; true; i++) {
        auto ch = text[i];
        if (ch == '\0') {
            //printf("TestImpl : %d, %d", i, alen);
            break;
        }

        if ((ch & 0x80) != 0x80) {
            // if (ch >> 7 == 0) {
            // if (ch  <= 0x7F) {

            //printf("ch = %c\n", ch);

        }
        else {
            // printf("jap ch = %c\n", ch);
            chs.push_back(ch);
        }
    }

    chs.push_back('\0');
    std::string parsed_text{ &chs[0] };

    EXPECT_EQ(chs.size(), 20);

    if (ARM) {
        // This might cause crash on old android devices
        //EXPECT_EQ(parsed_text, "😂日本語たち");
    }

    // printf("japanese text = %s, ", a);
}

ENDTEST

bool func(int, char) {
    return true;
}






TEST(ParserTest_, utf16Length) {

    // Table 3-8.  Use of U+FFFD in UTF-8 Conversion
    // http://www.unicode.org/versions/Unicode6.1.0/ch03.pdf

    {
        const char *str = u8"👨‍👩‍👧";
        int utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 8);
    }

    {
        const char *str = u8"😂abcd";
        int utf16length = ParseUtil::utf16_length(str, 4); // only for the first emoji
        EXPECT_EQ(utf16length, 2);
    }

    {
        const char *str = u8"nanimo-*";
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 8);
    }

    {
        const char *str = u8"a𐐀b";
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 4);
    }

    {
        const char *str = u8"\r\n\n"; // line break
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 3);
    }

    {
        const char *str = u8" "; // 1 space
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 1);
    }

    {
        const char *str = u8""; // empty string
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 0);
    }

    {
        const char *str = u8"Hasta el próximo miércoles"; // spanish
        auto utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 26);
    }

    {
        const char *str = u8"de 13.0 と Emoji 13.0 に準拠した 😀😁😂などの色々な表情の顔文字や 👿悪魔 👹鬼 👺天狗 👽エイリアン 👻おばけ 😺ネコの顔文字と💘❤💓💔💕💖ハ";
        int utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 96);
    }

    {
        const char *str = u8"我喜欢吃水果。Wǒ xǐhuan chī shuǐguǒ．私は果物が好きです。";
        int utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 39);
    }

    {
        const char *str = u8"안녕하세요";
        int utf16length = ParseUtil::utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 5);
    }


}

ENDTEST

/*
#define is_trail(c) (c > 0x7F && c < 0xC0)

#define SUCCESS 1
#define FAILURE -1

int  ConvChU32ToU16(const char32_t u32Ch) {
    if (u32Ch < 0 || u32Ch > 0x10FFFF) {
        //return -1;
        return 1;
    }

    if (u32Ch < 0x10000) {
        //u16Ch[0] = char16_t(u32Ch);
        //u16Ch[1] = 0;
        return 1;
    }
    else {
        //u16Ch[0] = char16_t((u32Ch - 0x10000) / 0x400 + 0xD800);
        //u16Ch[1] = char16_t((u32Ch - 0x10000) % 0x400 + 0xDC00);
        return 2;
    }

    //return true;
}

unsigned int utf8_get_next_char_or_ufffd(const unsigned char *str, unsigned int len, unsigned int *cursor)
{

    unsigned int pos = *cursor;
    unsigned int char_len = len - pos;
    unsigned char c;
    unsigned char min;
    unsigned char max;
    unsigned int code_point = 0xFFFD;

    c = str[pos];

    if (char_len < 1) {
        pos += 1;
    }
    else if (c < 0x80) {
        code_point = str[pos];
        pos += 1;
    }
    else if (c < 0xC2) {
        pos += 1;
    }
    else if (c < 0xE0) {

        if (char_len < 2 || !is_trail(str[pos + 1])) {
            pos += 1;
        }
        else {
            code_point = ((str[pos] & 0x1F) << 6) + (str[pos + 1] & 0x3F);
            pos += 2;
        }

    }
    else if (c < 0xF0) {

        min = (c == 0xE0) ? 0xA0 : 0x80;
        max = (c == 0xED) ? 0x9F : 0xBF;

        if (char_len < 2 || str[pos + 1] < min || max < str[pos + 1]) {
            pos += 1;
        }
        else if (char_len < 3 || !is_trail(str[pos + 2])) {
            pos += 2;
        }
        else {
            code_point = ((str[pos] & 0xF) << 12)
                | ((str[pos + 1] & 0x3F) << 6)
                | (str[pos + 2] & 0x3F);
            pos += 3;
        }

    }
    else if (c < 0xF5) {

        min = (c == 0xF0) ? 0x90 : 0x80;
        max = (c == 0xF4) ? 0x8F : 0xBF;

        if (len < 2 || str[pos + 1] < min || max < str[pos + 1]) {
            pos += 1;
        }
        else if (len < 3 || !is_trail(str[pos + 2])) {
            pos += 2;
        }
        else if (len < 4 || !is_trail(str[pos + 3])) {
            pos += 3;
        }
        else {
            code_point = ((str[pos] & 0x7) << 18)
                | ((str[pos + 1] & 0x3F) << 12)
                | ((str[pos + 2] & 0x3F) << 6)
                | (str[pos + 3] & 0x3F);
            pos += 4;
        }

    }
    else {
        pos += 1;
    }

    *cursor = pos;

    return code_point;
}
*/




/*
TEST(parser_test, utf8ToCodePoint) {

    // Table 3-8.  Use of U+FFFD in UTF-8 Conversion
     // http://www.unicode.org/versions/Unicode6.1.0/ch03.pdf
    //char *str = u8"😂日本語たち";// u8"nanimo-*";// u8"あいえおkん";// "👨‍👩‍👧";
    char *str = u8"👨‍👩‍👧";
    unsigned int code_point, len, cursor, cursor_before;

    len = strlen(str);
    cursor = 0;

    while (cursor < len) {
        cursor_before = cursor;
        code_point = utf8_get_next_char_or_ufffd((const unsigned char*)str, len, &cursor);
        printf("U+%X カーソル:%d\n", ConvChU32ToU16(code_point), cursor_before);
    }

    //EXPECT_LT(one_op_nanosec, 40000);
    //EXPECT_EQ(count, loopCount - 1);
    //EXPECT_EQ(ch, ' ');

}

ENDTEST
*/


TEST(ParserTest_, JustScanLetters) {

    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();
    uint64_t loopCount = 100 * 1000LLU;

    std::string text = u8R"(
class TestCl😂日本語10234ass {


}




class a {

}
class agiplkmp {

}

class jips {

}




  )";

    unsigned long long count = 0;
    const char *chars = text.c_str();
    char ch = 'a';
    for (unsigned long long i = 0; i < loopCount; i++) {
        if (i > 5) {
            for (uint_fast32_t k = 0; k < text.size(); k++) {
                ch = chars[k];
                if (ch == ' ') {
                    count = i;
                }
                else if (ch == 'a') {
                    count = i;
                }
            }
        }
    }

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    EXPECT_LT(one_op_nanosec, 40000);
    EXPECT_EQ(count, loopCount - 1);
    EXPECT_EQ(ch, ' ');

}

ENDTEST

int main3(int argc, char *argv[]) {
    return 0;
}

/*
TEST(parser_test, ParserStream) {
    FILE *fp;
    size_t rsize;
    int ret;
    static constexpr int BUFFER_SIZE = 10240;
    char buff[BUFFER_SIZE];

    fp = stdin;// fopen("ConsoleApplication2.pdb", "rb");

    if (fp == nullptr) {
        printf("failed to open file:errno=%d\n", errno);
        return;
    }

    int totalByteCount = 0;
    while (true) {
        rsize = fread(buff, 1, BUFFER_SIZE, fp);
        if (rsize > 0) {
            totalByteCount += rsize;

            printf("rsize=%d\n", rsize);
            continue;
        }

        break;
    }

    printf("total=%d\n", totalByteCount);


    if (feof(fp) == 0) {
        printf("failed to read file: errno=%d\n", errno);
        fclose(fp);
        return;
    }

    ret = fclose(fp);

    if (ret != 0) {
        printf("failed to close file: errno=%d\n", errno);
        return;
    }

    //EXPECT_EQ(3, 4);

}

ENDTEST
*/


TEST(ParserTest_, parser_benchmark) {


    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();
    uint64_t loopCount = 100 * 1000LLU;
    //std::string text = "   class           A   {    }   ";
    std::string text = u8R"(
class TestCl😂日本語10234ass {


}




class a {

}
class agiplkmp {

}

class jips {

}

 


  )";

    const char *chars = text.c_str();
    for (unsigned long long i = 0; i < loopCount; i++) {
        auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        //VTables::DocumentVTable->init((NodeBase*)&document);
        DocumentUtils::parseText(document, chars, text.size());

        //EXPECT_EQ(document->nodeCount, 4);
        Alloc::deleteDocument(document);
        //console_log("i:"+ std::to_string(i));
    }

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    //EXPECT_LT(one_op_nanosec, 15000);
    EXPECT_LT(one_op_nanosec, 35000);
    std::cout << "one" << one_op_nanosec;
}

ENDTEST


TEST(ParserTest_, aaHashMap) {


    {
        auto hashKey = HashMap::calc_hash2("ak", 10000);
        auto hashKey2 = HashMap::calc_hash2("ka", 10000);
        EXPECT_NE(hashKey, hashKey2);
    }

    {
        auto hashKey = HashMap::calc_hash2("N01", 10000);
        auto hashKey2 = HashMap::calc_hash2("N01234C", 10000);
        EXPECT_NE(hashKey, hashKey2);
    }

    auto *context = (ParseContext *)malloc(sizeof(ParseContext));
    context->memBuffer.init();

    for (int i = 0; i < 100; i++) {

        auto *hashMap = context->newMem<HashMap>();
        hashMap->init(context);

        auto *first = Cast::upcast(context->newMem<DocumentStruct>());
        const char key[] = "firstAA";
        hashMap->put2(key, Cast::upcast(context->newMem<DocumentStruct>()));
        hashMap->put2(key, first); // replace

        hashMap->put2("secondBB", Cast::upcast(context->newMem<DocumentStruct>()));
        hashMap->put2("jfiow", Cast::upcast(context->newMem<DocumentStruct>()));
        hashMap->put("jfiow", sizeof("jfiow") - 1, Cast::upcast(context->newMem<DocumentStruct>()));

        auto *node = hashMap->get2("firstAA");
        EXPECT_EQ(node, first);

        node = hashMap->get2("jfiow");
        EXPECT_EQ(node != nullptr, true);
        {
            auto *node = hashMap->get2("empty");
            EXPECT_EQ(node, nullptr);
        }

        //free(hashMap);
    }

}

ENDTEST

struct S {
    int a{ 3 };
};

TEST(ParserTest_, charBuffer) {
    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();
    uint64_t loopCount = 100 * 1000LLU;

    {
        srand((unsigned int)time(nullptr));
        for (int i = 0; i < 80 * 100; i++) {

            auto *charBuffer3 = (MemBuffer*)malloc(sizeof(MemBuffer));
            charBuffer3->init();
            
            unsigned int max = 1 + rand() % 10;
            for (int j = 0; j < max; j++) {
                unsigned int len = 1 + rand() % 100;

                auto *chars = charBuffer3->newMem<S>(len);
                chars->a = 5;

                auto *chars2 = charBuffer3->newMem<S>(1);
                chars2->a = 2;
                EXPECT_EQ(chars->a, 5);
                EXPECT_EQ(chars2->a, 2);

                EXPECT_EQ(charBuffer3->currentBufferBlock, *((MemBufferBlock **)((st_byte*)chars2 - st_size_of(MemBufferBlock*))));

                charBuffer3->tryDelete(chars);
                charBuffer3->tryDelete(chars2);

            }
            charBuffer3->freeAll();
            free(charBuffer3);
        }

    }

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    EXPECT_LT(one_op_nanosec, 40000);
}

ENDTEST


TEST(ParserTest_, CodeNode) {
    std::string text = "   class           A   {    }   ";
    /*
    std::string text = u8R"(
class A {
    class B {
        class TestCl😂日本語10234ass {

        }
    }
}
class A {}
class A {}
class A {}
class BDD{}




class AABC  {  }
)";
    */

    const char *chars = text.c_str();
    auto *document = Alloc::newDocument(
        DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Alloc::deleteDocument(document);

}

ENDTEST



TEST(ParserTest_, ErrorNodeTest_class) {
    std::string text = u8R"(
class A {
    class B {
        class TestCl😂日本語10234ass {

        }

        class C { }
    }
}
)";

    const char *chars = text.c_str();
    auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Alloc::deleteDocument(document);

}

ENDTEST



TEST(ParserTest_, ErrorNodeTest) {
    return;
    //std::string text = "   class           A   {    }   ";
    std::string text = u8R"(
class A {

    class B {
        @hoge(akaw=3242, ajwe=2342)
        class TestCl😂日本語10234ass {
            
        }

        cmpl fn fawe() {

        }
    }
}
class A {}
class A {}
class A {}
class BDD{}




class AABC  {  }
)";

    const char *chars = text.c_str();
    auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Alloc::deleteDocument(document);

}

ENDTEST


/*
TEST(parser_test, IndentTextTest) {
    std::string text = u8R"(
class A {
    class B {
        class TestCl😂日本語10234ass {

        }
    }
}
)";

    std::string textWithIndent = u8R"(
class A {
    class B {
        class TestCl😂日本語10234ass {
            func(32423, ()=>{
                var a = 2342;
            }, 432);
)";

    textWithIndent += "            ";

    textWithIndent += u8R"(
        }
    }
}
)";


    const char *chars = text.c_str();
    const char *indentChars = textWithIndent.c_str();
    auto *document = Allocator::newDocument(DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(indentChars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Allocator::deleteDocument(document);

}

ENDTEST
 */






    TEST(ParserTest_, ParseUtil) {

    EXPECT_EQ(true, ParseUtil::isIdentifierLetter('a'));

    ParseUtil::letterCheck(&func);



    EXPECT_EQ(true, ParseUtil::isIdentifierLetter('a'));
    EXPECT_EQ(true, ParseUtil::isIdentifierLetter(std::string{ u8"😂" }.c_str()[0]));
    EXPECT_EQ(false, ParseUtil::isIdentifierLetter('\n'));


    EXPECT_EQ(0, ParseUtil::matchFirstWithTrim("class A{}", "class"));

    EXPECT_EQ(-1, ParseUtil::matchFirstWithTrim("", "class"));
    EXPECT_EQ(-1, ParseUtil::matchFirstWithTrim("", ""));

    {
        std::string class_text(u8"     \tclassauto * 😂日本語=10234;");
        int index = ParseUtil::matchFirstWithTrim(class_text.c_str(), "class", 0);
        EXPECT_EQ(index, 6);
    }


    {
        std::string class_text(u8"😂classauto;");
        int index = ParseUtil::matchFirstWithTrim(class_text.c_str(), "class", 0);
        EXPECT_EQ(index, -1);
    }


    // matchWord
    {
        std::string class_text(u8"class");
        EXPECT_EQ(class_text.length(), 5);
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        EXPECT_EQ(result, true);
    }

    {
        std::string class_text(u8" class");
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        EXPECT_EQ(result, false);
    }

    {
        std::string class_text(u8"abcclass");
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 3);
        EXPECT_EQ(result, true);
    }

    {
        std::string class_text(u8"classauto;");
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        EXPECT_EQ(result, true);
    }

    {
        std::string text(u8"ab");
        auto result = ParseUtil::matchWord(text.c_str(), text.length(), "abcdefg", 5, 0);
        EXPECT_EQ(result, false);
    }

}

ENDTEST


