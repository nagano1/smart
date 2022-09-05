
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
#include <stdint.h>

#include "code_nodes.hpp"

namespace smart
{
    ErrorInfo ErrorInfo::ErrorInfoList[errorListSize];
    bool ErrorInfo::errorInfoInitialized{false};
    //static int _ab = initErrorInfoList();


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

    CodeLine *VTableCall::callAppendToLine(void *node, CodeLine *currentCodeLine) {
        if (node == nullptr) {
            return currentCodeLine;
        }
        auto *nodeBase = Cast::upcast(node);
        //if (nodeBase->prevLineBreakNode)
        return nodeBase->vtable->appendToLine(nodeBase, currentCodeLine);
    }


    // support nest
    int searchEndBlockCommentPos(int currentIdx, char *chars, int charLength) {
        int retPos = charLength;

        // /*
         /* */
        // */
        int commentStartPos = ParseUtil::indexOf2(chars, charLength, currentIdx, '/', '*');
        int commentEndPos = ParseUtil::indexOf2(chars, charLength, currentIdx, '*', '/');
        if (commentEndPos > -1) {
            if (commentStartPos > - 1 && commentStartPos < commentEndPos) { // nested block comment found
                int nestedClosePos = searchEndBlockCommentPos(commentStartPos + 2, chars, charLength);
                retPos = searchEndBlockCommentPos(nestedClosePos, chars, charLength);

            } else {
                retPos = commentEndPos + 2;
            }
        }

        return retPos;
    }

    int Scanner::scan_for_root(void *parentNode, // NOLINT(readability-function-cognitive-complexity)
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
        void *commentNode = nullptr;

        for (int32_t i = start; i <= context->length;) {
            ch = context->chars[i];
            // fprintf(stderr, "%c ,", ch);
            // fflush(stderr);
            // __android_log_print(ANDROID_LOG_DEBUG, "aaa", "here = %d,%c",i, ch);
            //console_log(("i:" + std::string(":") + ch + "," + std::to_string(i)).c_str());

            if (ch == '/') { // comment
                int commendEndIndex = -1;
                bool isLineComment = false;

                // line comment with "//"
                if ('/' == context->chars[i+1]) {
                    commendEndIndex = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, i);
                    isLineComment = true;

                } // block comment /* */
                else if ('*' == context->chars[i+1]) {
                    //  find the correspond "*/"
                    commendEndIndex = searchEndBlockCommentPos(i + 2, context->chars, context->length);
                }

                if (commendEndIndex > -1) {
                    auto *prevCommentNode = commentNode;

                    if (isLineComment) {
                        auto *comment = Alloc::newLineCommentNode(context, Cast::upcast(parentNode));
                        Init::assignText_SimpleTextNode(comment, context, i, commendEndIndex - i);

                        commentNode = comment;
                    }
                    else {
                        commentNode = generateBlockCommentFragments(parentNode, context, i,
                                                                    commendEndIndex);
                    }

                    NodeBase *comment2 = Cast::upcast(commentNode);
                    if (whitespace_startpos != -1 && whitespace_startpos < i) {
                        comment2->prev_chars = i - whitespace_startpos;
                        whitespace_startpos = -1;
                    }

                    if (prevCommentNode != nullptr) {
                        comment2->prevCommentNode = prevCommentNode;
                    }

                    if (prevLineBreak != nullptr) {
                        comment2->prevLineBreakNode = prevLineBreak;
                        prevLineBreak = nullptr;
                    }

                    i = commendEndIndex;
                    returnResult = i;
                    continue;
                }
            }
            else if (ParseUtil::isBreakLine(ch)) {
                context->afterLineBreak = true;
                auto *newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));

                if (prevLineBreak == nullptr) {
                    lastLineBreak = prevLineBreak = newLineBreak;
                }
                else {
                    lastLineBreak->nextLineBreakNode = newLineBreak;
                    lastLineBreak = newLineBreak;
                }

                if (whitespace_startpos != -1) {
                    if (whitespace_startpos < i) {
                        lastLineBreak->prev_chars = i - whitespace_startpos;
                    }
                    whitespace_startpos = -1;
                }

                if (commentNode != nullptr) {
                    newLineBreak->prevCommentNode = commentNode;
                    commentNode = nullptr;
                }


                bool rn = ch == '\r' && context->chars[i+1] == '\n';
                if (rn) { // \r\n
                    newLineBreak->text[0] = '\r';
                    newLineBreak->text[1] = '\n';
                    newLineBreak->text[2] = '\0';
                    i += 2;
                } else {
                    i++;
                }
                continue;
            }
            else if (ParseUtil::isSpace(ch)) {
                int spaceEndIndex = i + 1;
                for (; spaceEndIndex < context->length; spaceEndIndex++) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
                    if (!ParseUtil::isSpace(context->chars[spaceEndIndex])) {
                        break;
                    }
                }

                whitespace_startpos = i;
                i = spaceEndIndex;
                continue;
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

                // assign spaces
                if (context->leftNode != nullptr) {
                    if (whitespace_startpos != -1) {
                        context->leftNode->prev_chars = i - whitespace_startpos;
                        whitespace_startpos = -1;
                    }

                    if (commentNode != nullptr) {
                        context->leftNode->prevCommentNode = commentNode;
                        commentNode = nullptr;
                    }

                    context->leftNode->prevLineBreakNode = prevLineBreak;
                }

                i = result;

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
            //if (!root) {
                break;
            //}
            i++;
        }

        if (root) {
            context->remainedLineBreakNode = prevLineBreak;
            context->remainedCommentNode = commentNode;
            if (whitespace_startpos > -1 && whitespace_startpos < context->length) {
                context->remaindPrevChars = context->length - whitespace_startpos;
            }
        }
        context->scanEnd = false;
        return returnResult;
    }

    inline void *Scanner::generateBlockCommentFragments(void *parentNode, ParseContext *context,
                                           const int32_t &i, int commendEndIndex) {
        void *commentNode;
        auto *blockComment = Alloc::newBlockCommentNode(context, Cast::upcast(parentNode));

        int currentIndex = i;
        BlockCommentFragmentStruct *lastNode = nullptr;
        LineBreakNodeStruct *lastBreakLine = nullptr;

        while (currentIndex <= commendEndIndex) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
            int idxOfCommentEnd = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, currentIndex);

            if (commendEndIndex < idxOfCommentEnd) {
                idxOfCommentEnd = commendEndIndex;
            }

            if (idxOfCommentEnd > -1 && currentIndex <= idxOfCommentEnd) {
                auto *commentFragment = Alloc::newBlockCommentFragmentNode(context,
                                                          Cast::upcast(parentNode));

                commentFragment->prevLineBreakNode = lastBreakLine;

                int commentLength = idxOfCommentEnd - currentIndex;
                Init::assignText_SimpleTextNode(commentFragment, context, currentIndex, commentLength);
                //fprintf(stderr, "<idxOfCommentEnd: %d>", idxOfCommentEnd);

                auto *newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));
                bool rn = context->chars[idxOfCommentEnd] == '\r' && context->chars[idxOfCommentEnd+1] == '\n';
                if (rn) { // \r\n
                    newLineBreak->text[0] = '\r';
                    newLineBreak->text[1] = '\n';
                    newLineBreak->text[2] = '\0';
                    currentIndex = idxOfCommentEnd + 2;
                }
                else {
                    currentIndex = idxOfCommentEnd + 1;
                }

                lastBreakLine = newLineBreak;

                if (lastNode != nullptr) {
                    lastNode->nextNode = Cast::upcast(commentFragment);
                }
                lastNode = commentFragment;
                if (blockComment->firstCommentFragment == nullptr) {
                    blockComment->firstCommentFragment = commentFragment;
                }
            }
            else {
                break;
            }
        }
        commentNode = blockComment;
        return commentNode;
    }


    int Tokenizers::expressionTokenizer(TokenizerParams_parent_ch_start_context) {
        int result = numberTokenizer(TokenizerParams_pass);

        if (result == -1) result = boolTokenizer(TokenizerParams_pass);
        if (result == -1) result = nullTokenizer(TokenizerParams_pass);
        if (result == -1) result = parenthesesTokenizer(TokenizerParams_pass);
        if (result == -1) result = variableTokenizer(TokenizerParams_pass);
        if (result == -1) result = stringLiteralTokenizer(TokenizerParams_pass);

        if (result == -1) return -1;

        // access operator
        // pointer->val
        // "jfiowj".length


        // call func expression
        // func()
        int extraPos;
        if (-1 < (extraPos = Tokenizers::funcCallTokenizer(parent, context->chars[result],
                                                           result, context))) {
            result = extraPos;
        }

        //  binary operator expression
        // calc() + 421431
        if (-1 < (extraPos = Tokenizers::binaryOperationTokenizer(parent, context->chars[result],
                                                                  result, context))) {
            result = extraPos;
        }


        return result;
    }
}
