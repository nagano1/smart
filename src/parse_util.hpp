﻿#pragma once

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

struct ParseUtil {

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


    static inline bool matchWord(const utf8byte *text, st_size text_length, const char *word, st_size word_length, st_uint start) {
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
    static int matchFirstWithTrim(const char *chars, const char *target, int start) {
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


    static inline bool isSpaceOrLineBreak(utf8byte ch) {
        return ' ' == ch || ch == '\n' || '\t' == ch;
    };

    static inline bool isSpace(utf8byte ch) {
        return ' ' == ch || '\t' == ch;
    };

    static inline bool isNonIdentifierChar(utf8byte ch) {
        return ' ' == ch || '\t' == ch || '!' == ch || '#' == ch || '[' == ch || '\n' == ch
            || '%' == ch || ']' == ch || '"' == ch || '[' == ch || '\'' == ch
            || '=' == ch || '*' == ch || '+' == ch || '-' == ch || '?' == ch
            || '@' == ch || '{' == ch || '}' == ch || ',' == ch || ';' == ch
            || ':' == ch || '.' == ch || '`' == ch || '&' == ch || '|' == ch
            || '<' == ch || '>' == ch || '^' == ch || '\\' == ch || '/' == ch;
    };


    static inline bool isBreakLine(utf8byte ch) {
        return '\r' == ch || '\n' == ch;
    };

    static inline bool isNumberLetter(utf8byte ch) {
        return '0' <= ch && ch <= '9';
    }

    static inline bool isIdentifierLetter(utf8byte ch) {
        if ('A' <= ch && ch <= 'Z') {
            return true;
        }
        else if ('a' <= ch && ch <= 'z') {
            return true;
        }
        else if ('0' <= ch && ch <= '9') {
            return true;
        }
        else if ('_' == ch) {
            return true;
        }

        return (ch & 0x80) == 0x80;
    };
};

