#include <stdio.h>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>

#include "code_nodes.hpp"

namespace smart {
    ErrorInfo ErrorInfoList[errorListSize];
    bool errorInfoInitialized{false};
    static int ab = initErrorInfoList();

    int HashMap::calc_hash(const char *key, int keyLength, size_t max) {
        unsigned int sum = keyLength;
        int border = 128;

        int salt = 0; // prevent same result from only order different of letters
        for (int i = 0; i < keyLength && i < border; i++) {
            unsigned char unsignedValue = key[i];
            sum += unsignedValue;
            salt += i % 2 == 0 ? unsignedValue * i : -unsignedValue * i;
        }

        for (int i = keyLength-1,j=0; i >= border && j < border; i--,j++) {
            unsigned char unsignedValue = key[i];// (key[i] < 0 ? -key[i] : key[i]);
            sum += unsignedValue;
            salt += j % 2 == 0 ? unsignedValue * j : -unsignedValue * j;
        }
        if (salt < 0) {
            salt = -(salt);
        }
        return (sum + salt) % max;
    }

    void HashMap::put(const char *keyA, st_textlen keyLength, NodeBase* val) {

        auto hashInt = calc_hash(keyA, keyLength, this->entries_length);
        HashNode* hashNode = this->entries[hashInt];

        if (hashNode == nullptr) {// || hashNode->key == nullptr) {
            auto *newHashNode = context->newMem<HashNode>();
            newHashNode->next = nullptr;
            this->entries[hashInt] = newHashNode;


            char *keyB = context->memBuffer.newMem<char>(keyLength + 1);
            for (st_uint i = 0; i < keyLength; i++) {
                keyB[i] = keyA[i];
            }
            newHashNode->key = keyB;
            newHashNode->keyLength = keyLength;
            newHashNode->nodeBase = val;
            return;
        }

        while (true) {
            // find same key
            if (hashNode->keyLength == keyLength) {
                bool sameKey = true;
                for (st_uint i = 0; i < keyLength; i++) {
                    if (hashNode->key[i] != keyA[i]) {
                        sameKey = false;
                        break;
                    }
                }
                if (sameKey) {
                    hashNode->nodeBase = val;
                    return;
                }
            }

            if (hashNode->next == nullptr) {
                break;
            }
            hashNode = hashNode->next;
        }


        auto *newHashNode = context->newMem<HashNode>();
        char *keyB =  context->newMemArray<char>(keyLength + 1);
        for (st_uint i = 0; i < keyLength; i++) {
            keyB[i] = keyA[i];
        }
        newHashNode->key = keyB;
        newHashNode->keyLength = keyLength;
        newHashNode->nodeBase = val;
        newHashNode->next = nullptr;

        hashNode->next = newHashNode;
    }


    void HashMap::init(ParseContext *context) {
        this->context = context;
        this->entries = (HashNode**)context->newMemArray<HashNode>(HashNode_TABLE_SIZE);
        this->entries_length = HashNode_TABLE_SIZE;

        memset(this->entries, 0, sizeof(HashNode*)*this->entries_length);
        for (unsigned int i = 0; i < this->entries_length; i++) {
            this->entries[i] = nullptr;
        }
    }

    bool HashMap::has(const char * key, int keyLength) {
        return this->entries[calc_hash0(key, keyLength)]->key != nullptr;
    }

    void HashMap::deleteKey(const char * key, int keyLength) {
        if (this->entries[calc_hash0(key, keyLength)] != nullptr) {
            free(this->entries[calc_hash0(key, keyLength)]);
        }
    }

    NodeBase* HashMap::get(const char * key, st_textlen keyLength) {
        auto keyInt = calc_hash0(key, keyLength);
        if (this->entries[keyInt] != nullptr) {
            auto * hashNode = this->entries[keyInt];
            while (hashNode) {
                if (hashNode->keyLength == keyLength) {
                    bool sameKey = true;
                    for (st_uint i = 0; i < keyLength; i++) {
                        if (hashNode->key[i] != key[i]) {
                            sameKey = false;
                            break;
                        }
                    }

                    if (sameKey) {
                        return hashNode->nodeBase;
                    }
                }
                
                hashNode = hashNode->next;
            }
        }
        return nullptr;
    }




    int Scanner::scanErrorNodeUntilSpace(
        void *parentNode,
        int start,
        ParseContext *context
    ) {
        return 3;
    }


    int Scanner::scan(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context
    ) {
        return Scanner::scan_for_root(parentNode, tokenizer, start, context, false);
    }

    CodeLine *VTableCall::appendToLine(void *node, CodeLine *currentCodeLine) {
        if (node == nullptr) {
            return currentCodeLine;
        }
        auto *nodeBase = Cast::upcast(node);
        return nodeBase->vtable->appendToLine(nodeBase, currentCodeLine);
    }


    static SpaceNodeStruct *genSpaceNode(ParseContext *context, void *parentNode, int start, int end) {

        auto *prevSpaceNode = Alloc::newSpaceNode(context, Cast::upcast(parentNode));
        prevSpaceNode->text =  context->memBuffer.newMem<char>(end - start + 1);
        memcpy(prevSpaceNode->text, context->chars + start, (end - start));
        prevSpaceNode->textLength = end - start;
/*
        for (int i = 0; i < end - start; i++) {
            prevSpaceNode->text[i] = ' ';
        }
        prevSpaceNode->text[(end - start)] = '\0';
*/

        return prevSpaceNode;
    }


    int Scanner::scan_for_root(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context,
        bool root
    ) {
        LineBreakNodeStruct *prevLineBreak = nullptr;
        LineBreakNodeStruct *lastLineBreak = nullptr;

        utf8byte ch;
        int returnResult = -1;

        int32_t whitespace_startpos = -1;

        //context->scanEnd = false;
        for (uint32_t i = start; i < context->length;) {
            ch = context->chars[i];
            // fprintf(stderr, "%c ,", ch);
            // fflush(stderr);
            // __android_log_print(ANDROID_LOG_DEBUG, "aaa", "here = %d,%c",i, ch);
            // console_log(("i:" + std::string(":") + ch + "," + std::to_string(i)).c_str());

            if (ParseUtil::isBreakLine(ch)) {
                context->afterLineBreak = true;
                auto *newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));

                if (prevLineBreak == nullptr) {
                    lastLineBreak = prevLineBreak = newLineBreak;
                }
                else {
                    lastLineBreak->nextLineBreakNode = newLineBreak;
                    lastLineBreak = newLineBreak;
                }

                if (whitespace_startpos != -1 && (uint32_t)whitespace_startpos < i) {
                    lastLineBreak->prevSpaceNode = genSpaceNode(context, parentNode, whitespace_startpos, i);
                    whitespace_startpos = -1;
                }

                i++;
                continue;
            }
            else if (ParseUtil::isSpace(ch)) {
                uint32_t spaceEndIndex = i + 1;

                for (; spaceEndIndex < context->length; spaceEndIndex++) {
                    if (!ParseUtil::isSpace(context->chars[spaceEndIndex])) {
                        break;
                    }
                }

                whitespace_startpos = i;
                i = spaceEndIndex;
                continue;
            }

            if (ch == '\0') {
                break;
            }

            int result = tokenizer(Cast::upcast(parentNode), ch, i, context);
            context->afterLineBreak = false;

            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            if (result > -1) {
                //console_log(":try:" + std::to_string(result));

                // Attach a space node
                if (whitespace_startpos != -1) {
                    if (context->chars[whitespace_startpos] == ' ' && i - whitespace_startpos == 1) {
                        context->codeNode->prev_char = ' ';
                    }
                    else {
                        context->codeNode->prevSpaceNode = genSpaceNode(context, parentNode, whitespace_startpos, i);
                    }

                    whitespace_startpos = -1;
                }

                returnResult = i = result;

                context->codeNode->prevLineBreakNode = prevLineBreak;
                prevLineBreak = nullptr;
                lastLineBreak = nullptr;

                if (context->scanEnd) {
                    context->scanEnd = false;
                    break;
                }
                continue;
            }


            if ((ch & 0x80) != 0x80) {

            }

            if (!root) {
                break;
            }

            i++;
        }

        if (root) {
            context->remainedLineBreakNode = prevLineBreak;
            if (whitespace_startpos > -1 && (uint32_t)whitespace_startpos < context->length) {
                context->remainedSpaceNode = genSpaceNode(context, parentNode,
                    whitespace_startpos, context->length);
            }
        }
        else {
            if (prevLineBreak) {
                //delete prevLineBreak;
            }
        }

        //context->former_start = start;
        return returnResult;
    }
}