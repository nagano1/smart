#pragma once

#include <iostream>
#include <string>
#include <condition_variable>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>

#include "common.hpp"

using letterCheckerType = bool(*)(int, char);


/*

UTF-8

0xxxxxxx                            0 - 127
110yyyyx 10xxxxxx                   128 - 2047
1110yyyy 10yxxxxx 10xxxxxx          2048 - 65535
11110yyy 10yyxxxx 10xxxxxx 10xxxxxx 65536 - 0x10FFFF

at least one of the y should be 1
*/

/*
UTF-16

codepoint 	UTF-16
xxxxxxxxxxxxxxxx         	xxxxxxxxxxxxxxxx
000uuuuuxxxxxxxxxxxxxxxx 	110110wwwwxxxxxx 110111xxxxxxxxxx 	wwww = uuuuu - 1
*/
static constexpr unsigned char utf8BytesTable[256]{
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


/*
48	0x30	0
65	0x41	A
97	0x61	a
*/
static constexpr unsigned char hex_asciicode_table[256]{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //0 
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,// 32
    0,1,2,3,4,5,6,7,8,9,1,1,1,1,1,1,// 48
    1,10,11,12,13,14,15,16,1,1,1,1,1,1,1,1, //64
    10,11,12,13,14,15,16,1,1,1,1,1,1,1,1,1, // 80
    1,10,11,12,13,14,15,16,1,1,1,1,1,1,1,1, // 96
    10,11,12,13,14,15,16,1,1,1,1,1,1,1,1,1, // 112
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 128
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 144
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 160
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 176
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 192
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
};
struct ParseUtil {
    // \u8e60
    // \0061 -> a
    static int parseUtf16toUtf8(const char* utf16_chars, unsigned int len, int index, int *consumed,
        unsigned char* ch1, unsigned  char* ch2, unsigned char* ch3, unsigned char* ch4) {

        // \u6382
        if (index + 6 > (int)len) {
            return 0;
        }

        const unsigned char* chars = (unsigned char*)(utf16_chars + index);

        assert(chars[0] == '\\');
        assert(chars[1] == 'u');

        int ch = (hex_asciicode_table[(int)chars[2]] << 4) | hex_asciicode_table[(int)chars[3]];
        bool hasPair = (ch >> 2) == 0x36; // 110110ww
        
        if (hasPair) {
            // \uD840\uDFF9
            if (index + 12 > (int)len) {
                return 0;
            }

            assert(chars[6] == '\\');
            assert(chars[7] == 'u');

            int codePoint = (ch << 8)
                | ((int)hex_asciicode_table[(int)chars[4]] << 4)
                | (int)hex_asciicode_table[(int)chars[5]];
            codePoint = codePoint & 0b0000001111111111;

            int codePoint2 =
                ((int)hex_asciicode_table[(int)chars[8]] << 12)
                | ((int)hex_asciicode_table[(int)chars[9]] << 8)
                | ((int)hex_asciicode_table[(int)chars[10]] << 4)
                | (int)hex_asciicode_table[(int)chars[11]];
            codePoint2 = codePoint2 & 0b0000001111111111;


            // 𠏹
            // {\"fwe\": \"\uD840\uDFF9\"}
            // 
            // &#x203F9;
            // &#132089;
            // URL - encoded UTF8 % F0 % A0 % 8F % B9

            // utf16 uuuuuxxxxxxxxxxxxxxxx 	110110wwww_xxxx_xx 110111xxxx_xxxxxx 	(wwww = uuuuu - 1)
            // utf8 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx 65536 - 0x10FFFF
            int left1 = (codePoint >> 6) + 1;
            *ch1 = (left1 >> 3) | 0b11110000;
            *ch2 = ((left1 & 0b11) << 4) | ((codePoint >> 2) & 0b1111) | 0x80;
            *ch3 = ((codePoint & 0b11) << 4) | (codePoint2 >> 6) | 0x80;
            *ch4 = (codePoint2 & 0b111111) | 0x80;
            *consumed = 12;
            return 4;
        }
        else {
            int codePoint = (ch << 8)
                | ((int)hex_asciicode_table[(int)chars[4]] << 4)
                | (int)hex_asciicode_table[(int)chars[5]];

            printf("codepoint = [%x]\n", codePoint);

            // v = 1 << (sizeof(unsigned int)*8 - 1);
            *consumed = 6;

            if (codePoint < 128) { // 0xxxxxxx 0 - 127
                *ch1 = (unsigned char)codePoint;
                return 1;
            }
            else if (codePoint < 2048) { // 110yyyyx 10xxxxxx 128 - 2047
                *ch1 = codePoint>>6 | 0xC0;
                *ch2 = (codePoint & 0x3F) | 0x80;
                return 2;
            } 
            else if (codePoint < 65536) { // 1110yyyy 10yxxxxx 10xxxxxx 2048 - 65535
                *ch1 = codePoint >> 12 | 0b11100000;
                *ch2 = ((codePoint & 0b111111000000) >> 6) | 0x80;
                *ch3 = ((codePoint & 0b111111)) | 0x80;

                return 3;
            }
        }

        return 0;
    }



    static int utf16_length(const char *utf8_chars, unsigned int byte_len) {
        unsigned int pos = 0;
        int length = 0;

        while (pos < byte_len) {
            auto idx = (unsigned char)utf8_chars[pos];
            int bytes = utf8BytesTable[idx];
            pos += bytes;
            length += bytes > 3 ? 2 : 1;
        }
        return length;
    }


    template<class T>
    static inline int detectOne(const T &tokenizer, const utf8byte *chars, utf8byte ch, int i) {
        if (tokenizer.first_char == ch) {
            auto idx = tokenizer.tryTokenize(chars, i);
            if (idx > -1) {
                return idx;
            }
        }

        return -1;
    };

    template<class T>
    static inline int detect(const T &tokenizer, const utf8byte *chars, utf8byte ch, int i) {
        for (int k = 0; k < tokenizer.first_chars_length; k++) {
            if (tokenizer.first_chars[k] == ch) {
                auto idx = tokenizer.tryTokenize(chars, i);
                if (idx > -1) {
                    return idx;
                }
            }
        }

        return -1;
    };


    static inline bool letterCheck(letterCheckerType letterChecker) {
        return letterChecker(3, 'b');
    };

    static inline int
        matchFirstWithTrim(const std::string &&class_text, const std::string &&target) {
        return ParseUtil::matchFirstWithTrim(class_text.c_str(), target.c_str(), 0);
    };


    static inline bool matchWord(const utf8byte *text,
        st_size text_length,
        const char *word, st_size word_length,
        st_uint start)
    {
        if (start + word_length <= text_length) { // determine word has enough length
            for (st_uint i = 0; i < word_length; i++) {
                if (text[start + i] != word[i]) {
                    return false;
                }
            }
            return true;
        }

        return false;
    }



    // EXPECT_EQ(0, Tokenizer::matchFirstWithTrim("class A{}", "class"));
    static int matchFirstWithTrim(const char *chars, const char *target, int start)
    {
        //  return -1 if it fails
        int currentTargetIndex = 0;
        int matchStartIndex = -1;

        for (int i = start; true; i++) {
            auto ch = chars[i];

            if ((ch & 0x80) != 0x80) {
                if (ch == '\0') {
                    return -1;
                }

                if (matchStartIndex == -1) {
                    if (ch == ' ') { //allow trim
                        continue;
                    }
                    else if (ch == '\t' || ch == '\n' || ch == '\r') {
                        continue;
                    }
                    else { // should start match
                        matchStartIndex = i;
                        currentTargetIndex = 0;
                    }
                }

                if (target[currentTargetIndex] == '\0') {
                    break;
                }

                if (target[currentTargetIndex] == chars[i]) {
                    currentTargetIndex++;
                    continue;
                }
            }

            return -1;
        }

        if (currentTargetIndex == 0) {
            return -1;
        }
        else {
            return matchStartIndex;
        }
    };


    static inline bool isSpaceOrLineBreak(utf8byte ch)
    {
        return ' ' == ch || ch == '\n' || '\t' == ch;
    }


    static inline bool isSpace(utf8byte ch)
    {
        return ' ' == ch || '\t' == ch;
    }


    static inline bool isNonIdentifierChar(utf8byte ch)
    {
        return ch == ' ' || '\t' == ch || '!' == ch || '#' == ch || '\n' == ch
               || '%' == ch || ']' == ch || '"' == ch || '[' == ch || '\'' == ch
               || '=' == ch || '*' == ch || '+' == ch || '-' == ch || '?' == ch
               || '@' == ch || '{' == ch || '}' == ch || ',' == ch || ';' == ch
               || ':' == ch || '.' == ch || '`' == ch || '&' == ch || '|' == ch
               || '<' == ch || '>' == ch || '^' == ch || '\\' == ch || '/' == ch;
    }


    static inline bool isBreakLine(utf8byte ch)
    {
        return '\r' == ch || '\n' == ch;
    }


    static inline bool isNumberLetter(utf8byte ch)
    {
        return '0' <= ch && ch <= '9';
    }


    static inline bool isIdentifierLetter(utf8byte ch)
    {
        if ('A' <= ch && ch <= 'Z') {
            return true;
        } else if ('a' <= ch && ch <= 'z') {
            return true;
        }
        else if ('0' <= ch && ch <= '9') {
            return true;
        }
        else if ('_' == ch) {
            return true;
        }

        return (ch & 0x80) == 0x80;
    }
};

