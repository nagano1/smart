#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "tokenizer.hpp"
#include "../test_common.h"

using namespace smart;


TEST(parser_test, JsonParseTest) {

    char *text = const_cast<char *>(u8R"(
{"jsonrpc":"2.0","id":0,"method":"initialize","params":{"processId":28196,"rootPath":null,"rootUri":null,"capabilities":{"workspace":{"applyEdit":true,"workspaceEdit":{"documentChanges":true},"didChangeConfiguration":{"dynamicRegistration":true},"didChangeWatchedFiles":{"dynamicRegistration":true},"symbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"executeCommand":{"dynamicRegistration":true},"configuration":true,"workspaceFolders":true},"textDocument":{"publishDiagnostics":{"relatedInformation":true},"synchronization":{"dynamicRegistration":true,"willSave":true,"willSaveWaitUntil":true,"didSave":true},"completion":{"dynamicRegistration":true,"contextSupport":true,"completionItem":{"snippetSupport":true,"commitCharactersSupport":true,"documentationFormat":["markdown","plaintext"],"deprecatedSupport":true},"completionItemKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}},"hover":{"dynamicRegistration":true,"contentFormat":["markdown","plaintext"]},"signatureHelp":{"dynamicRegistration":true,"signatureInformation":{"documentationFormat":["markdown","plaintext"]}},"definition":{"dynamicRegistration":true},"references":{"dynamicRegistration":true},"documentHighlight":{"dynamicRegistration":true},"documentSymbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"codeAction":{"dynamicRegistration":true},"codeLens":{"dynamicRegistration":true},"formatting":{"dynamicRegistration":true},"rangeFormatting":{"dynamicRegistration":true},"onTypeFormatting":{"dynamicRegistration":true},"rename":{"dynamicRegistration":true},"documentLink":{"dynamicRegistration":true},"typeDefinition":{"dynamicRegistration":true},"implementation":{"dynamicRegistration":true},"colorProvider":{"dynamicRegistration":true}}},"trace":"off","workspaceFolders":null}}
)");

    text = const_cast<char *>(u8R"({
)");

    auto *document = Allocator::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, text, strlen(text));

    char *treeText = DocumentUtils::getTextFromTree(document);
    if (treeText != nullptr) {
        Allocator::deleteDocument(document);
        // EXPECT_EQ(std::string(treeText), "\n{b:18}\n");
        //EXPECT_EQ(std::string{ treeText }, std::string{ text });
    }

}

ENDTEST


/*
 test of iterating chars over utf8 text
 Point here is just whether you can distinguish ascii code from others even if it contains a wide char like emoji
*/
TEST(parser_test, char_iteration) {

    std::string wstr{ u8"auto * 😂日本語たち=10234;" };
    std::string wstr2{ "class TestClass{ }" };
    int alen = wstr.length();
    auto chs = std::vector<char>{};

    //chs.push_back(0xEF);
    //chs.push_back(0xBB);
    //chs.push_back(0xBF);

    //std::cout << "\n[" << wstr.length() << "\n";

    auto &&val = std::move(23);

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




/*

UTF-8

0xxxxxxx                            0 - 127
110yyyyx 10xxxxxx                   128 - 2047
1110yyyy 10yxxxxx 10xxxxxx          2048 - 65535
11110yyy 10yyxxxx 10xxxxxx 10xxxxxx 65536 - 0x10FFFF

at least one of the y should be 1
*/

static const unsigned char utf8BytesTable[256]{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
};


int utf16_length(const char *utf8_chars
                 , unsigned int byte_len) {
    unsigned int pos = 0;
    int length = 0;

    while (pos < byte_len) {
        auto idx = (unsigned char) utf8_chars[pos];
        int bytes = utf8BytesTable[idx];
        pos += bytes;
        length += bytes > 3 ? 2 : 1;
    }
    return length;
}


TEST(parser_test, utf16Length) {

    // Table 3-8.  Use of U+FFFD in UTF-8 Conversion
    // http://www.unicode.org/versions/Unicode6.1.0/ch03.pdf

    {
        const char *str = u8"👨‍👩‍👧";
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 8);
    }

    {
        const char *str = u8"😂abcd";
        int utf16length = utf16_length(str, 4); // only for the first emoji
        EXPECT_EQ(utf16length, 2);
    }

    {
        const char *str = u8"nanimo-*";
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 8);
    }

    {
        const char *str = u8"a𐐀b";
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 4);
    }

    {
        const char *str = u8"\r\n\n"; // line break
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 3);
    }

    {
        const char *str = u8" "; // 1 space
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 1);
    }

    {
        const char *str = u8""; // empty string
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 0);
    }

    {
        const char *str = u8"Hasta el próximo miércoles"; // spanish
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 26);
    }

    {
        const char *str = u8"de 13.0 と Emoji 13.0 に準拠した 😀😁😂などの色々な表情の顔文字や 👿悪魔 👹鬼 👺天狗 👽エイリアン 👻おばけ 😺ネコの顔文字と💘❤💓💔💕💖ハ";
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 96);
    }

    {
        const char *str = u8"我喜欢吃水果。Wǒ xǐhuan chī shuǐguǒ．私は果物が好きです。";
        int utf16length = utf16_length(str, strlen(str));
        EXPECT_EQ(utf16length, 39);
    }

    {
        const char *str = u8"안녕하세요";
        int utf16length = utf16_length(str, strlen(str));
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


TEST(parser_test, JustScanLetters) {

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


TEST(parser_test, parser_benchmark) {


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
        auto *document = Allocator::newDocument(DocumentType::CodeDocument, nullptr);
        //VTables::DocumentVTable->init((NodeBase*)&document);
        DocumentUtils::parseText(document, chars, text.size());

        //EXPECT_EQ(document->nodeCount, 4);
        Allocator::deleteDocument(document);
        //console_log("i:"+ std::to_string(i));
    }

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    EXPECT_LT(one_op_nanosec, 15000);
    std::cout << "one" << one_op_nanosec;
}

ENDTEST



TEST(parser_test, charBuffer) {
    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();
    uint64_t loopCount = 100 * 1000LLU;

    CharBuffer<char> charBuffer;
    charBuffer.firstBufferList = nullptr;
    charBuffer.currentBufferList = nullptr;
    charBuffer.spaceNodeIndex = INT16_MAX;
    
    
    auto *chars = charBuffer.newChars(255);
    EXPECT_EQ(charBuffer.firstBufferList, charBuffer.currentBufferList);

    auto *chars2 = charBuffer.newChars(1);
    EXPECT_NE(charBuffer.firstBufferList, charBuffer.currentBufferList);


    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    EXPECT_LT(one_op_nanosec, 40000);
}

ENDTEST


TEST(parser_test, CodeNode) {
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
    auto *document = Allocator::newDocument(
                           DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Allocator::deleteDocument(document);

}

ENDTEST



TEST(parser_test, ErrorNodeTest_class) {
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
    auto *document = Allocator::newDocument(DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Allocator::deleteDocument(document);

}

ENDTEST



TEST(parser_test, ErrorNodeTest) {
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
    auto *document = Allocator::newDocument(DocumentType::CodeDocument, nullptr);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    EXPECT_EQ(std::string(treeText), std::string(chars));
    EXPECT_EQ(strlen(treeText), strlen(chars));

    Allocator::deleteDocument(document);

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


    TEST(parser_test, Tokenizer) {

    EXPECT_EQ(true, Tokenizer::isIdentifierLetter('a'));

    Tokenizer::letterCheck(&func);

    EXPECT_EQ(true, Tokenizer::isIdentifierLetter('a'));
    EXPECT_EQ(true, Tokenizer::isIdentifierLetter(std::string{ u8"😂" }.c_str()[0]));
    EXPECT_EQ(false, Tokenizer::isIdentifierLetter('\n'));


    EXPECT_EQ(0, Tokenizer::matchFirstWithTrim("class A{}", "class"));

    EXPECT_EQ(-1, Tokenizer::matchFirstWithTrim("", "class"));
    EXPECT_EQ(-1, Tokenizer::matchFirstWithTrim("", ""));

    {
        std::string class_text(u8"     \tclassauto * 😂日本語=10234;");
        int index = Tokenizer::matchFirstWithTrim(class_text.c_str(), "class", 0);
        EXPECT_EQ(index, 6);
    }


    {
        std::string class_text(u8"😂classauto;");
        int index = Tokenizer::matchFirstWithTrim(class_text.c_str(), "class", 0);
        EXPECT_EQ(index, -1);
    }

}

ENDTEST


