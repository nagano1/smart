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

    void HashMap::put(const char *keyA, int keyLength, NodeBase* val) const {

        auto hashInt = calc_hash(keyA, keyLength, this->entries_length);
        HashNode* hashNode = this->entries[hashInt];

        if (hashNode == nullptr) {// || hashNode->key == nullptr) {
            auto *newHashNode = context->newMem<HashNode>();
            newHashNode->next = nullptr;
            this->entries[hashInt] = newHashNode;


            char *keyB = context->memBuffer.newMem<char>(keyLength + 1);
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
            if (hashNode->keyLength == keyLength) {
                bool sameKey = true;
                for (int i = 0; i < keyLength; i++) {
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
        for (int i = 0; i < keyLength; i++) {
            keyB[i] = keyA[i];
        }
        newHashNode->key = keyB;
        newHashNode->keyLength = keyLength;
        newHashNode->nodeBase = val;
        newHashNode->next = nullptr;

        hashNode->next = newHashNode;
    }


    void HashMap::init(ParseContext *ctx) {
        this->context = ctx;
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

    NodeBase* HashMap::get(const char * key, int keyLength) {
        auto keyInt = calc_hash0(key, keyLength);
        if (this->entries[keyInt] != nullptr) {
            auto * hashNode = this->entries[keyInt];
            while (hashNode) {
                if (hashNode->keyLength == keyLength) {
                    bool sameKey = true;
                    for (int i = 0; i < keyLength; i++) {
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



    int Scanner::scanOnce(void *parentNode,
                      TokenizerFunction tokenizer,
                      int start,
                      ParseContext *context
    ) {
        return Scanner::scan_for_root(parentNode, tokenizer, start, context, false, false);
    }


    // scan until scanEnd==true
    int Scanner::scanMulti(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context
    ) {
        return Scanner::scan_for_root(parentNode, tokenizer, start, context, false, true);
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

    // support nest
    static int searchEndBlockCommentPos(int currentIdx, char *chars, int charLength) {
        int idxEnd = charLength;

        // /*
         /* */
        // */
        int commentStartPos = ParseUtil::indexOf2(chars, charLength, currentIdx, '/', '*');
        int commentEndPos = ParseUtil::indexOf2(chars, charLength, currentIdx, '*', '/');
        if (commentEndPos > -1) {
            if (commentStartPos > - 1 && commentStartPos < commentEndPos) { // nested block comment found
                int nestedClosePos = searchEndBlockCommentPos(commentStartPos + 2, chars, charLength);
                idxEnd = searchEndBlockCommentPos(nestedClosePos, chars, charLength);

            } else {
                idxEnd = commentEndPos + 2;
            }
        }

        return idxEnd;
    }

    int Scanner::scan_for_root(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context,
        bool root, bool scanMulti
    ) {
        LineBreakNodeStruct *prevLineBreak = nullptr;
        LineBreakNodeStruct *lastLineBreak = nullptr;

        utf8byte ch;
        int returnResult = -1;

        int32_t whitespace_startpos = -1;
        LineCommentNodeStruct *commentNode = nullptr;

        //context->scanEnd = false;
        for (int32_t i = start; i <= context->length;) {
            ch = context->chars[i];
//             fprintf(stderr, "%c ,", ch);
//             fflush(stderr);
            // __android_log_print(ANDROID_LOG_DEBUG, "aaa", "here = %d,%c",i, ch);
            //console_log(("i:" + std::string(":") + ch + "," + std::to_string(i)).c_str());


            if (ch == '/') {
                int idxEnd = -1;
                bool isLineComment = false;

                // line comment with "//"
                if ('/' == context->chars[i+1]) {
                    idxEnd = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, i);
                    isLineComment = true;

                } // block comment /* */
                else if ('*' == context->chars[i+1]) {
                    // try to find "*/"
                    idxEnd = searchEndBlockCommentPos(i + 2, context->chars, context->length);
                }

                if (idxEnd > -1) {
                    auto *prevCommentNode = commentNode;

                    if (isLineComment) {
                        commentNode = Alloc::newLineCommentNode(context, Cast::upcast(parentNode));
                    }
                    else {
                        commentNode = Alloc::newBlockCommentNode(context, Cast::upcast(parentNode));
                    }


                    Init::assignText_SimpleTextNode(commentNode, context, i, idxEnd - i);

                    if (whitespace_startpos != -1 && whitespace_startpos < i) {
                        commentNode->prevSpaceNode = genSpaceNode(context, parentNode, whitespace_startpos, i);

                        if (prevCommentNode != nullptr) {
                            commentNode->prevSpaceNode->prevBlockCommentNode = prevCommentNode;
                        }

                        whitespace_startpos = -1;
                    } else {
                        if (prevCommentNode != nullptr) {
                            commentNode->prevBlockCommentNode = prevCommentNode;
                        }
                    }

                    i = idxEnd;
                    continue;
                }
            }

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

                if (whitespace_startpos != -1 && whitespace_startpos < i) {
                    auto *spaceNode = genSpaceNode(context, parentNode, whitespace_startpos, i);
                    lastLineBreak->prevSpaceNode = spaceNode;

                    if (commentNode != nullptr) {
                        spaceNode->prevBlockCommentNode = commentNode;
                        commentNode = nullptr;
                    }

                    whitespace_startpos = -1;
                } else {
                    if (commentNode != nullptr) {
                        newLineBreak->prevBlockCommentNode = commentNode;
                        commentNode = nullptr;
                    }
                }

                bool rn = ch == '\r' && context->chars[i+1] == '\n';
                bool isLastChar = i ==  context->length - 1;
                if (rn) { // \r\n
                    newLineBreak->text[0] = '\r';
                    newLineBreak->text[1] = '\n';
                    newLineBreak->text[2] = '\0';
                    i += 2;
                } else {
                    i++;
                }
                if (!isLastChar) {
                    continue;
                }
            }
            else if (ParseUtil::isSpace(ch)) {
                int spaceEndIndex = i + 1;

                for (; spaceEndIndex < context->length; spaceEndIndex++) {
                    if (!ParseUtil::isSpace(context->chars[spaceEndIndex])) {
                        break;
                    }
                }

                whitespace_startpos = i;
                bool isLastChar = i ==  context->length - 1;
                i = spaceEndIndex;
                if (!isLastChar) {
                    continue;
                }
            }



            int result = tokenizer(Cast::upcast(parentNode), ch, i, context);

            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            returnResult = result;
            if (result > -1) {
                context->afterLineBreak = false;

                context->prevFoundPos = result;
                //console_log(":try:" + std::to_string(result));

                // Attach a space node
                if (whitespace_startpos != -1) {
                    if (context->codeNode != nullptr) {
                        if (context->chars[whitespace_startpos] == ' ' &&
                            i - whitespace_startpos == 1) {
                            context->codeNode->prev_char = ' ';

                            if (commentNode != nullptr) {
                                context->codeNode->prevBlockCommentNode = commentNode;
                                commentNode = nullptr;
                            }

                        } else {
                            context->codeNode->prevSpaceNode = genSpaceNode(context, parentNode,
                                                                            whitespace_startpos, i);

                            if (commentNode != nullptr) {
                                context->codeNode->prevSpaceNode->prevBlockCommentNode = commentNode;
                                commentNode = nullptr;
                            }
                        }
                    }
                    whitespace_startpos = -1;
                }

                returnResult = i = result;

                if (context->codeNode != nullptr) {
                    context->codeNode->prevLineBreakNode = prevLineBreak;
                }
                prevLineBreak = nullptr;
                lastLineBreak = nullptr;

                if (context->scanEnd) {
                    context->scanEnd = false;
                    break;
                }
                if (scanMulti) {
                    continue;
                }
            }
            if (ch == '\0') {
                break;
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
            if (whitespace_startpos > -1 && whitespace_startpos < context->length) {
                context->remainedSpaceNode = genSpaceNode(context, parentNode,
                    whitespace_startpos, context->length);
            }
        }
        else {
            if (prevLineBreak) {
                //delete prevLineBreak;
            }
        }
        context->scanEnd = false;

        //context->former_start = start;
        return returnResult;
    }
}