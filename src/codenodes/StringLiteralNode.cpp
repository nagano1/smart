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

    static CodeLine *appendToLine(StringLiteralNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const utf8byte *self_text(StringLiteralNodeStruct *self) {
        return self->text;
    }

    static st_textlen selfTextLength(StringLiteralNodeStruct *self) {
        return self->textLength;
    }


    int Tokenizers::stringLiteralTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;

        // starts with "
        bool startsWithDQuote = false;
        bool endsWithDQuote = false;

        if (ch == '"') {
            startsWithDQuote = true;
            found_count++;
        }
        else {
            return -1;
        }

        int letterStart = startsWithDQuote ? start + 1 : start;
        bool escapeMode = false;

        for (uint_fast32_t i = letterStart; i < context->length; i++) {
            /*if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            }
            else 
            */
            if (startsWithDQuote) {
                found_count++;

                if (escapeMode) {
                    escapeMode = false;
                    continue;
                }

                if (context->chars[i] == '\\') {
                    escapeMode = true;
                    continue;
                }

                if (context->chars[i] == '"') {
                    endsWithDQuote = true;
                    break;
                }
            } else {
                break;
            }
        }





        if (startsWithDQuote && !endsWithDQuote) {
            context->syntaxErrorInfo.setError(&context->syntaxErrorInfo,10234, start, "no end quote");
            /*
            context->syntaxErrorInfo.hasError = true;
            context->syntaxErrorInfo.charPosition = start;
            context->syntaxErrorInfo.reason = (const char*)"no end quote";
            context->syntaxErrorInfo.errorCode = 21390;
*/
            return -1;
        }


        if (found_count > 0) {
            auto *strLiteralNode = context->newMem<StringLiteralNodeStruct>();
            Init::initStringLiteralNode(strLiteralNode, context, parent);
            context->codeNode = Cast::upcast(strLiteralNode);

            strLiteralNode->text = context->memBuffer.newMem<char>(found_count + 1);

            strLiteralNode->textLength = found_count;

            memcpy(strLiteralNode->text, context->chars + start, found_count);
            strLiteralNode->text[found_count] = '\0';


            // create actual string
            auto *str = context->memBuffer.newMem<char>(found_count+3);
            bool escapeMode = false;
            int strLength = 0;
            int currentStrIndex = 0;
            for (uint_fast32_t i = 1; i < found_count-1; i++) {
                /* \uXXXX		4s, 16unit Unicode char */
                if (escapeMode) {
                    escapeMode = false;
                    auto ch = strLiteralNode->text[i];
                    when(ch) {
                        wfor_noop('r');
                        wfor('n', str[currentStrIndex++] = '\n');
                        wfor('t', str[currentStrIndex++] = '\t');
                        wfor('\\', str[currentStrIndex++] = '\\');
                        wfor('f', str[currentStrIndex++] = 'f');
                        wfor('/', str[currentStrIndex++] = '/');
                        wfor('"', str[currentStrIndex++] = '"');
                        wfor('\'', str[currentStrIndex++] = '\'');
                        wfor('u', str[currentStrIndex++] = 'u');
                        welse(str[currentStrIndex++] = strLiteralNode->text[i]);
                    }
                    continue;
                }


                if (strLiteralNode->text[i] == '\\') {
                    escapeMode = true;
                }
                else {
                    auto ch = strLiteralNode->text[i];
                    str[currentStrIndex++] = ch;
                }

                strLength++;
            }

            if (startsWithDQuote) {
                strLiteralNode->literalType = 0;

                strLiteralNode->str = str;
                strLiteralNode->strLength = strLength;
                strLiteralNode->str[strLength] = '\0';

            }
            else {
                strLiteralNode->literalType = 1;
                
                strLiteralNode->str = strLiteralNode->text;
                strLiteralNode->strLength = strLiteralNode->textLength;
            }

            
            return start + found_count;
        }

        return -1;

    };

    static constexpr const char nameTypeText[] = "<string>";

    static const node_vtable _VTable = CREATE_VTABLE(StringLiteralNodeStruct, selfTextLength,
                                                          self_text,
                                                          appendToLine, nameTypeText, false);
    const node_vtable *VTables::StringLiteralVTable = &_VTable;

    void Init::initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::StringLiteralVTable);
        name->text = nullptr;
        name->textLength = 0;
        name->str = nullptr;
        name->strLength = 0;

    }
}