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


    int HashMap::calc_hash(char *key, int keyLength) {
        unsigned int sum = keyLength;
        int border = HashNode_TABLE_SIZE;

        int salt = 0; // prevent same result from only order different of letters
        for (int i = 0; i < keyLength && i < border; i++) {
            unsigned char unsignedValue = key[i];// < 0 ? -key[i] : key[i]);
            sum += unsignedValue;
            salt += i % 2 == 0 ? unsignedValue * i : -unsignedValue * i;
        }

        for (int i = keyLength-1,j=0; i >= border && j < HashNode_TABLE_SIZE ; i--,j++) {
            unsigned char unsignedValue = key[i];// (key[i] < 0 ? -key[i] : key[i]);
            sum += unsignedValue;
            salt += j % 2 == 0 ? unsignedValue * j : -unsignedValue * j;
        }
        if (salt < 0) {
            salt = -(salt);
        }
        return (sum + salt) % HashNode_TABLE_SIZE;
    }

    void HashMap::put(char * keyA, int keyLength, NodeBase* val) {

        auto hashInt = calc_hash(keyA, keyLength);
        HashNode* hashNode = this->entries[hashInt];

        if (hashNode == nullptr) {// || hashNode->key == nullptr) {
            auto *newHashNode = simpleMalloc<HashNode>();
            newHashNode->next = nullptr;
            this->entries[hashInt] = newHashNode;


            char *keyB = charBuffer.newChars(keyLength + 1);
            for (int i = 0; i < keyLength; i++) {
                keyB[i] = keyA[i];
            }
            newHashNode->key = keyB;
            newHashNode->keyLength = keyLength;
            newHashNode->nodeBase = val;
            return;
        }

        while (true) {
            // find same key
            bool sameKey = true;
            if (hashNode->keyLength == keyLength) {
                for (int i = 0; i < keyLength; i++) {
                    if (hashNode->key[i] != keyA[i]) {
                        sameKey = false;
                        break;
                    }
                }
            }
            else {
                sameKey = false;
            }

            if (sameKey) {
                hashNode->nodeBase = val;
                return;
            }

            if (hashNode->next == nullptr) {
                break;
            }
            hashNode = hashNode->next;
        }


        auto *newHashNode = simpleMalloc<HashNode>();
        char *keyB = charBuffer.newChars(keyLength + 1);
        for (int i = 0; i < keyLength; i++) {
            keyB[i] = keyA[i];
        }
        newHashNode->key = keyB;
        newHashNode->keyLength = keyLength;
        newHashNode->nodeBase = val;
        newHashNode->next = nullptr;

        hashNode->next = newHashNode;
    }


    void HashMap::init() {
        charBuffer.init();
        this->entries = (HashNode**)malloc(sizeof(HashNode*)*HashNode_TABLE_SIZE);
        /*
        memset(this->entries, 0, sizeof(this->entries));
        
        for (int i = 0; i < HashNode_TABLE_SIZE; i++) {
            this->entries[i] = nullptr;

            if (this->entries[i] != nullptr) {
                throw 3;
            }
        }
        */
        for (int i = 0; i < HashNode_TABLE_SIZE; i++) {
            this->entries[i] = nullptr;
        }
        for (int i = 0; i < HashNode_TABLE_SIZE; i++) {
            if (this->entries[i] != nullptr) {
                //throw 3;
            }
        }
    }

    bool HashMap::has(char * key, int keyLength) {
        return this->entries[calc_hash(key, keyLength)]->key != nullptr;
    }

    void HashMap::deleteKey(char * key, int keyLength) {
        if (this->entries[calc_hash(key, keyLength)] != nullptr) {
            free(this->entries[calc_hash(key, keyLength)]);
        }
    }

    NodeBase* HashMap::get(char * key, int keyLength) {
        auto keyInt = calc_hash(key, keyLength);
        if (this->entries[keyInt] != nullptr) {
            auto * hashNode = this->entries[keyInt];
            while (hashNode) {
                if (0 == strcmp(hashNode->key, key)) {
                    return hashNode->nodeBase;
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
        if (nodeBase->vtable == nullptr) {
            int a = 0;
        }
        return nodeBase->vtable->appendToLine(nodeBase, currentCodeLine);
    }


    static SpaceNodeStruct *
        genSpaceNode(ParseContext *context, void *parentNode, int start, int end) {

        auto *prevSpaceNode = Alloc::newSpaceNode(context, Cast::upcast(parentNode));
        prevSpaceNode->text = context->charBuffer.newChars(end - start + 1);
        memcpy(prevSpaceNode->text, context->chars + start, (end - start));
        prevSpaceNode->textLength = end - start;

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

        bool afterLineBreak = false;

        //context->scanEnd = false;
        for (uint32_t i = start; i < context->length;) {
            ch = context->chars[i];
            //__android_log_print(ANDROID_LOG_DEBUG, "aaa", "here = %d,%c",i, ch);
            //console_log("i:" + std::string(":") + ch + "," + std::to_string(i));

            if (ParseUtil::isBreakLine(ch)) {
                afterLineBreak = true;
                auto *newLineBreak
                    = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));

                if (prevLineBreak == nullptr) {
                    lastLineBreak = prevLineBreak = newLineBreak;
                }
                else {
                    lastLineBreak->nextLineBreakNode = newLineBreak;
                    lastLineBreak = newLineBreak;
                }

                if (whitespace_startpos != -1 && (uint32_t)whitespace_startpos < i) {
                    lastLineBreak->prevSpaceNode = genSpaceNode(context,
                        parentNode,
                        whitespace_startpos, i);
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
            afterLineBreak = false;

            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            if (result > -1) {
                //console_log(":try:" + std::to_string(result));

                // Attach a space node
                /*
*/
                if (whitespace_startpos != -1) {
                    if (context->chars[whitespace_startpos] == ' '
                        && i - whitespace_startpos == 1) {
                        context->codeNode->prev_char = ' ';
                    }
                    else {
                        context->codeNode->prevSpaceNode = genSpaceNode(context, parentNode,
                            whitespace_startpos, i);
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

        context->former_start = start;
        return returnResult;
    }
}