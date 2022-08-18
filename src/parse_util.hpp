#pragma once
/*
#include <string>
#include <condition_variable>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
*/
#include "common.hpp"

using letterCheckerType = bool(*)(int, char);


struct ParseUtil {
    // \u8e60
    static int parseJsonUtf16Sequense(const char* utf16_chars, unsigned int len, int index, int *consumed,
        unsigned char* ch1, unsigned  char* ch2, unsigned char* ch3, unsigned char* ch4);



    static int utf16_length(const char *utf8_chars, unsigned int byte_len);

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

    /*
    static inline int
        matchFirstWithTrim(const std::string &&class_text, const std::string &&target) {
        return ParseUtil::matchAt(class_text.c_str(), target.c_str(), 0);
    };
*/

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



    // EXPECT_EQ(0, Tokenizer::matchAt("class A{}", "class"));
    static int _matchFirstWithTrim(const char *chars, int charsLength, const char *target, int start);

    template<int SIZE>
    static int matchAt(const char *chars, int charsLength, int startIndex, const char(&target)[SIZE])
    {
        int pos = _matchFirstWithTrim(chars, charsLength, target, startIndex);

        if (pos > -1) {
            if (startIndex + SIZE - 1 < charsLength
                && ParseUtil::isTerminatableChar(chars[startIndex + SIZE - 1])
                    ) {
                return pos;
            }
        }

        return -1;
    }

    static bool hasCharBeforeLineBreak(const char *chars, int charsLength, int startIndex) {
        for (int i = startIndex; i < charsLength; i++) {
            if (chars[i] == ' ' || chars[i] == '\r') {
                continue;
            } else  if (chars[i] == '\n' || chars[i] == '\0') {
                return false;
            } else {
                return true;
            }
        }

        return false;
    }


    static inline bool isTerminatableChar(utf8byte ch)
    {
        return ' ' == ch || ch == '\n' || '\t' == ch
            || '/' == ch || '\r' == ch
            || ')' == ch || '}' == ch || ']' == ch
            || ',' == ch || '.' == ch;
    }


    static inline int indexOfBreakOrEnd(const char *chars, int charsLength, int startIndex)
    {
        for (int i = startIndex; i < charsLength; i++) {
            if ('\r' == chars[i] || '\0' == chars[i] || '\n' == chars[i]) {
                return i;
            }
        }
        return charsLength;
    }

    static inline int indexOf(const char *chars, int charsLength, int startIndex, char ch)
    {
        for (int i = startIndex; i < charsLength; i++) {
            if (ch == chars[i]) {
                return i;
            }
        }
        return -1;
    }

    static inline int indexOf2(const char *chars, int charsLength, int startIndex, char ch1, char ch2)
    {
        for (int i = startIndex; i < charsLength - 1; i++) {
            if (ch1 == chars[i] && ch2 == chars[i+1]) {
                return i;
            }
        }
        return -1;
    }



    static inline bool isSpace(utf8byte ch)
    {
        return ' ' == ch; // || '\t' == ch;
    }


    static inline bool endsWith(const char* chars, int charsLength, const char* suffix, int suffixLength)
    {
        if (charsLength < suffixLength) {
            return false;
        }

        for (int i = 0; i < suffixLength; i++) {
            int idx = charsLength - (suffixLength - i);
            if (chars[idx] != suffix[i]) {
                return false;
            }
        }

        return true;
    }

    static inline bool equal(const char* chars, int charsLength, const char* suffix, int suffixLength) {
        if (charsLength == suffixLength) {
            return endsWith(chars, charsLength, suffix, suffixLength);
        }
        return false;
    }


    template<int SIZE>
    static inline bool endsWith2(const char* chars, int charsLength, const char(&suffix)[SIZE])
    {
        int suffixLength = SIZE - 1;
        return endsWith(chars, charsLength, suffix, suffixLength);
    }


    static inline bool isNonIdentifierChar(utf8byte ch)
    {
        return ch == ' ' || '\t' == ch || '!' == ch || '#' == ch || '\n' == ch
               || '%' == ch || ']' == ch || '"' == ch || '[' == ch || '\'' == ch
               || '=' == ch || '*' == ch || '+' == ch || '-' == ch || '?' == ch
               || '@' == ch || '{' == ch || '}' == ch || ',' == ch || ';' == ch
               || ':' == ch || '.' == ch || '`' == ch || '&' == ch || '|' == ch
               || '<' == ch || '>' == ch || '^' == ch || '\\' == ch || '/' == ch
               || '\0' == ch;
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

