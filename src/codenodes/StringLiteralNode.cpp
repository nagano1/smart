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
        auto *node = self;
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
            context->syntaxErrorInfo.hasError = true;
            context->syntaxErrorInfo.charPosition = start;
            context->syntaxErrorInfo.reason = (char*)"no end quote";
            context->syntaxErrorInfo.errorCode = 21390;

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
            auto *str = context->memBuffer.newMem<char>(found_count);
            bool escapeMode = false;
            int strLength = 0;
            int currentStrIndex = 0;
            for (uint_fast32_t i = 1; i < found_count-1; i++) {

                /*
                \"	"	ダブルクォーテーション
\\	\	バックスラッシュ
\/	/	スラッシュ
\b		バックスペース
\f		改ページ
\n		キャリジリターン(改行)
\r		ラインフィード
\t		タブ
\uXXXX		4桁の16進数で表記されたUnicode文字
                */
                if (escapeMode) {
                    escapeMode = false;
                    auto ch = strLiteralNode->text[i];
                    if (ch == 'r') {
                        //str[currentStrIndex++] = '\r';
                    }
                    else if (ch == 'n') {
                        str[currentStrIndex++] = '\n';
                    }
                    else if (ch == 't') {
                        str[currentStrIndex++] = '\t';
                    }
                    else if (ch == '\\') {
                        str[currentStrIndex++] = '\\';
                    }
                    else if (ch == 'f') {
                        str[currentStrIndex++] = 'f';
                    }
                    else if (ch == '/') {
                        str[currentStrIndex++] = '/';
                    }
                    else if (ch == 'u') {
                        str[currentStrIndex++] = 'u'; // 
                    }
                    else {
                        str[currentStrIndex++] = strLiteralNode->text[i];

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