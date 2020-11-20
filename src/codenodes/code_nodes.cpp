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


    static SpaceNodeStruct *
    genSpaceNode(ParseContext *context, void *parentNode, int start, int end) {

        auto *prevSpaceNode = Allocator::newSpaceNode(context, Cast::upcast(parentNode));
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

        //context->scanEnd = false;
        for (uint32_t i = start; i < context->length;) {
            ch = context->chars[i];
            //__android_log_print(ANDROID_LOG_DEBUG, "aaa", "here = %d,%c",i, ch);
            //console_log("i:" + std::string(":") + ch + "," + std::to_string(i));

            if (Tokenizer::isBreakLine(ch)) {
                auto *newLineBreak
                        = Allocator::newLineBreakNode(context, Cast::upcast(parentNode));

                if (prevLineBreak == nullptr) {
                    lastLineBreak = prevLineBreak = newLineBreak;
                } else {
                    lastLineBreak->nextLineBreakNode = newLineBreak;
                    lastLineBreak = newLineBreak;
                }

                if (whitespace_startpos != -1 && (uint32_t) whitespace_startpos < i) {
                    lastLineBreak->prevSpaceNode = genSpaceNode(context,
                                                                parentNode,
                                                                whitespace_startpos, i);
                    whitespace_startpos = -1;
                }

                i++;
                continue;
            }

            if (Tokenizer::isSpace(ch)) {
                uint32_t spaceEndIndex = i + 1;

                for (; spaceEndIndex < context->length; spaceEndIndex++) {
                    if (!Tokenizer::isSpace(context->chars[spaceEndIndex])) {
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
            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            if (result > -1) {
                //console_log(":try:" + std::to_string(result));

                // Attach a space node
                if (whitespace_startpos != -1) {
                    if (context->chars[whitespace_startpos] == ' '
                        && i - whitespace_startpos == 1) {
                        context->codeNode->prev_char = ' ';
                    } else {
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
            if (whitespace_startpos > -1 && (uint32_t) whitespace_startpos < context->length) {
                context->remainedSpaceNode = genSpaceNode(context, parentNode,
                                                          whitespace_startpos, context->length);
            }
        } else {
            if (prevLineBreak) {
                //delete prevLineBreak;
            }
        }

        context->former_start = start;
        return returnResult;
    }
}