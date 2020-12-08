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

using letterCheckerType = bool (*)(int, char);

struct Tokenizer {

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
        return Tokenizer::matchFirstWithTrim(class_text.c_str(), target.c_str(), 0);
    };


    // EXPECT_EQ(0, Tokenizer::matchFirstWithTrim("class A{}", "class"));
    static int matchFirstWithTrim(const utf8byte *chars, const char *target, int start) {
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
                    } else if (ch == '\t' || ch == '\n' || ch == '\r') {
                        continue;
                    } else { // should start match
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
        } else {
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
        return ' ' == ch || '\t' == ch || '!' == ch || '#' == ch || '[' == ch
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
        } else if ('a' <= ch && ch <= 'z') {
            return true;
        } else if ('0' <= ch && ch <= '9') {
            return true;
        } else if ('_' == ch) {
            return true;
        }

        return (ch & 0x80) == 0x80;
    };

};

