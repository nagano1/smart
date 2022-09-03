#include <cstdio>
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

    static CodeLine *appendToLine(StringLiteralNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const utf8byte *self_text(StringLiteralNodeStruct *self) {
        return self->text;
    }

    static int selfTextLength(StringLiteralNodeStruct *self) {
        return self->textLength;
    }


    int Tokenizers::stringLiteralTokenizer(TokenizerParams_parent_ch_start_context) {
        int found_count = 0;

        // starts with "
        bool startsWithQuote = false;
        bool endsWithQuote = false;


        char quote;

        if (ch == '"') {
            startsWithQuote = true;
            found_count++;
            quote = '"';
        } else if (ch == '`'){
            startsWithQuote = true;
            found_count++;
            quote = '`';
        } else {
            return -1;
        }



        {
            int letterStart = (startsWithQuote) ? start + 1 : start;
            bool escapeMode = false;

            for (int_fast32_t i = letterStart; i < context->length; i++) {
                found_count++;

                if (escapeMode) {
                    escapeMode = false;
                    continue;
                }

                if (context->chars[i] == '\\') {
                    escapeMode = true;
                    continue;
                }

                if (startsWithQuote) {
                    if (context->chars[i] == quote) {
                        endsWithQuote = true;
                        break;
                    }
                }

//                if (startsWithDQuote) {
//                } else {
//                    break;
//                }
            }

        }

        if (startsWithQuote &&  !endsWithQuote) {
            context->setError(ErrorCode::missing_closing_quote, start);
            return -1;
        }


        if (found_count > 0) {
            auto *strLiteralNode = context->newMem<StringLiteralNodeStruct>();
            Init::initStringLiteralNode(strLiteralNode, context, parent);
            context->setCodeNode(strLiteralNode);

            strLiteralNode->text = context->memBuffer.newMem<char>(found_count + 1);

            strLiteralNode->textLength = found_count;

            memcpy(strLiteralNode->text, context->chars + start, found_count);
            strLiteralNode->text[found_count] = '\0';

            // create actual string
            auto *str = context->memBuffer.newMem<char>(found_count+ 3);
            bool escapeMode = false;
            int strLength = 0;
            int currentStrIndex = 0;
            for (int_fast32_t i = 1; i < found_count-1; i++) {
                /* \uXXXX		4s, 16unit Unicode char */

                if (escapeMode) {
                    escapeMode = false;
                    if (strLiteralNode->text[i] == 'u') {
                        int consumed = 0;

                        int utf16length = ParseUtil::parseJsonUtf16Sequense(strLiteralNode->text, strLiteralNode->textLength
                            , i - 1, &consumed
                            , (unsigned char*)&str[currentStrIndex]
                            , (unsigned char*)&str[currentStrIndex+1]
                            , (unsigned char*)&str[currentStrIndex+2]
                            , (unsigned char*)&str[currentStrIndex+3]);

                        if (utf16length > 0) {
                            currentStrIndex += utf16length;
                            strLength += utf16length;
                            i += consumed - 2;
                            continue;
                        }
                    }

                    when(strLiteralNode->text[i]) {
                        //wfor_noop('r')
                        wfor('r', str[currentStrIndex++] = '\r')
                        wfor('n', str[currentStrIndex++] = '\n')
                        wfor('t', str[currentStrIndex++] = '\t')
                        wfor('\\', str[currentStrIndex++] = '\\')
                        wfor('f', str[currentStrIndex++] = 'f')
                        wfor('/', str[currentStrIndex++] = '/')
                        wfor('"', str[currentStrIndex++] = '"')
                        wfor('\'', str[currentStrIndex++] = '\'')
                        wfor('u', str[currentStrIndex++] = 'u')
                        welse(str[currentStrIndex++] = strLiteralNode->text[i])
                    }

                    strLength++;

                    continue;
                }


                if (strLiteralNode->text[i] == '\\') {
                    escapeMode = true;
                } else {
                    strLength++;
                    str[currentStrIndex++] = strLiteralNode->text[i];
                }
            }

            if (startsWithQuote) {
                strLiteralNode->literalType = quote == '"' ? 0 : 1;
                strLiteralNode->str = str;
                strLiteralNode->strLength = strLength;
                strLiteralNode->str[strLength] = '\0';

            }

            return start + found_count;
        }

        return -1;

    }

    static constexpr const char nameTypeText[] = "<string>";

    static int applyFuncToDescendants(
            StringLiteralNodeStruct *node, void *targetVTable,
            int (*func)(NodeBase *, void *, void *, void *, void *)
            , void *arg, void *arg2)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), targetVTable, (void *)func, arg, arg2);
        }

        return 0;
    }

    static node_vtable _stringVTable = CREATE_VTABLE(StringLiteralNodeStruct, selfTextLength,
                                                          self_text,
                                                          appendToLine, applyFuncToDescendants, nameTypeText, NodeTypeId::StringLiteral);
    const node_vtable *VTables::StringLiteralVTable = &_stringVTable;

    void Init::initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::StringLiteralVTable);
        name->text = nullptr;
        name->textLength = 0;
        name->str = nullptr;
        name->strLength = 0;

    }
}