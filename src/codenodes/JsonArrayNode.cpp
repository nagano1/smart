#include <stdio.h>
#include <iostream>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <vector>

#include <cstdint>
#include <ctime>
#include <string.h>

#include "code_nodes.hpp"

namespace smart {

    enum phase {
        EXPECT_VALUE = 0,
        COMMA = 3
    };






    // -----------------------------------------------------------------------------------
    //
    //                              JsonArrayItemStruct
    //
    // -----------------------------------------------------------------------------------
    static CodeLine *appendToLine2(JsonArrayItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::appendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    };


    static const utf8byte *selfText_JsonKeyValueItemStruct(JsonArrayItemStruct *self) {
        return "";
    }

    static st_textlen selfTextLength2(JsonArrayItemStruct *self) {
        return 0;
    }


    static const node_vtable _jsonArrayItemVTable = CREATE_VTABLE(JsonArrayItemStruct,
                                                                  selfTextLength2,
                                                                  selfText_JsonKeyValueItemStruct,
                                                                  appendToLine2, "<JsonArrayItem>");

    const struct node_vtable *VTables::JsonArrayItemVTable = &_jsonArrayItemVTable;

















    // -----------------------------------------------------------------------------------
    //
    //                              JsonArrayStruct
    //
    // -----------------------------------------------------------------------------------

    static const utf8byte *selfText(JsonArrayStruct *node) {
        return "[";
    }

    static st_textlen selfTextLength(JsonArrayStruct *self) {
        return 1;
    }

    static constexpr const char _typeName[] = "<JsonArray>";

    static CodeLine *appendToLine(JsonArrayStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        currentCodeLine->parentDepth += 1;

        JsonArrayItemStruct *item = self->firstItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<JsonArrayItemStruct*>(item->nextNode);
        }

        currentCodeLine->parentDepth -= 1;

        return VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
    };



    static const node_vtable _VTable = CREATE_VTABLE(JsonArrayStruct,
        selfTextLength, selfText,
        appendToLine, _typeName);
    const struct node_vtable *VTables::JsonArrayVTable = &_VTable;







    JsonArrayStruct *Alloc::newJsonArray(ParseContext *context, NodeBase *parentNode) {
        auto *jsonArrayNode = context->newMem<JsonArrayStruct>();
        INIT_NODE(jsonArrayNode, context, parentNode, VTables::JsonArrayVTable);
        jsonArrayNode->firstItem = nullptr;
        jsonArrayNode->lastItem = nullptr;
        jsonArrayNode->parsePhase = phase::EXPECT_VALUE;

        Init::initSymbolNode(&jsonArrayNode->endBodyNode, context, Cast::upcast(jsonArrayNode), ']');

        return jsonArrayNode;
    }


    void Alloc::deleteJsonArray(NodeBase *node) {
        auto *classNode = Cast::downcast<JsonArrayStruct*>(node);

        //if (classNode->nameNode.name != nullptr) {
        //free(classNode->nameNode.name);
        //classNode->nameNode.name = nullptr;
        //}

        free(classNode);
    }



    static int internal_JsonArrayTokenizer(TokenizerParams_parent_ch_start_context);

    int Tokenizers::jsonArrayTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '[') {
            int returnPosition = start + 1;
            auto *jsonArray = Alloc::newJsonArray(context, parent);
            int result = Scanner::scan(jsonArray,
                internal_JsonArrayTokenizer,
                returnPosition,
                context);

            if (result > -1) {
                context->codeNode = Cast::upcast(jsonArray);

                returnPosition = result;
                return returnPosition;

            }
        }

        return -1;
    }





    JsonArrayItemStruct *Alloc::newJsonArrayItem(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<JsonArrayItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_jsonArrayItemVTable);

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }


    static inline void appendRootNode(JsonArrayStruct *arr, JsonArrayItemStruct *arrayItem) {
        assert(arr != nullptr && arrayItem != nullptr);

        if (arr->firstItem == nullptr) {
            arr->firstItem = arrayItem;
        }
        if (arr->lastItem != nullptr) {
            arr->lastItem->nextNode = Cast::upcast(arrayItem);
        }
        arr->lastItem = arrayItem;
    }



    int internal_JsonArrayTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *jsonArray = Cast::downcast<JsonArrayStruct *>(parent);

        if (ch == ']') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&jsonArray->endBodyNode);
            return start + 1;
        }

        if (jsonArray->parsePhase == phase::EXPECT_VALUE) {
            int result;
            if (-1 < (result = Tokenizers::jsonValueTokenizer(parent, ch, start, context))) {
                auto *nextItem = Alloc::newJsonArrayItem(context, parent);

                nextItem->valueNode = context->codeNode;

                appendRootNode(jsonArray, nextItem);

                jsonArray->parsePhase = phase::COMMA;
                //context->scanEnd = false;
                return result;
            }
            return -1;
        }

        auto *currentKeyValueItem = jsonArray->lastItem;

        if (jsonArray->parsePhase == phase::COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->codeNode = Cast::upcast(&currentKeyValueItem->follwingComma);
                jsonArray->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            }
            return -1;
        }

        return -1;
    }

}
