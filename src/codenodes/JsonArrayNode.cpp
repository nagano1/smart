#include <cstdio>
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
#include <cstring>

#include "code_nodes.hpp"

namespace smart {

    enum phase {
        EXPECT_VALUE = 0,
        EXPECT_COMMA = 3
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


    static const utf8byte *selfText_JsonKeyValueItemStruct(JsonArrayItemStruct *) {
        return "";
    }

    static int selfTextLength2(JsonArrayItemStruct *) {
        return 0;
    }


    static const node_vtable _jsonArrayItemVTable = CREATE_VTABLE(JsonArrayItemStruct,
                                                                  selfTextLength2,
                                                                  selfText_JsonKeyValueItemStruct,
                                                                  appendToLine2,
                                                                  "<JsonArrayItem>",
                                                                  NodeTypeId::JsonArrayItem);

    const struct node_vtable *const VTables::JsonArrayItemVTable = &_jsonArrayItemVTable;


    JsonArrayItemStruct *Alloc::newJsonArrayItem(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<JsonArrayItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_jsonArrayItemVTable);

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }











    // -----------------------------------------------------------------------------------
    //
    //                              JsonArrayStruct
    //
    // -----------------------------------------------------------------------------------

    static const utf8byte *selfText(JsonArrayStruct *) {
        return "[";
    }

    static int selfTextLength(JsonArrayStruct *) {
        return 1;
    }

    static constexpr const char _typeName[] = "<JsonArray>";

    static CodeLine *appendToLine(JsonArrayStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        self->context->parentDepth += 1;

        JsonArrayItemStruct *item = self->firstItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<JsonArrayItemStruct*>(item->nextNode);
        }

        self->context->parentDepth -= 1;

        return VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
    }



    static const node_vtable _jsonArrayVTable = CREATE_VTABLE(JsonArrayStruct,
                                                              selfTextLength, selfText,
                                                              appendToLine, _typeName, NodeTypeId::JsonArrayStruct);
    const struct node_vtable *const VTables::JsonArrayVTable = &_jsonArrayVTable;







    JsonArrayStruct *Alloc::newJsonArray(ParseContext *context, NodeBase *parentNode) {
        auto *jsonArrayNode = context->newMem<JsonArrayStruct>();
        INIT_NODE(jsonArrayNode, context, parentNode, VTables::JsonArrayVTable);
        jsonArrayNode->firstItem = nullptr;
        jsonArrayNode->lastItem = nullptr;
        jsonArrayNode->parsePhase = phase::EXPECT_VALUE;

        Init::initSymbolNode(&jsonArrayNode->endBodyNode, context, Cast::upcast(jsonArrayNode), ']');

        return jsonArrayNode;
    }



    static int internal_JsonArrayTokenizerMulti(TokenizerParams_parent_ch_start_context);

    int Tokenizers::jsonArrayTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '[') {
            int returnPosition = start + 1;
            auto *jsonArray = Alloc::newJsonArray(context, parent);
            jsonArray->parsePhase = phase::EXPECT_VALUE;

            int result = Scanner::scanMulti(jsonArray,
                                            internal_JsonArrayTokenizerMulti,
                                            returnPosition,
                                            context);
            if (result > -1) {
                context->setCodeNode(jsonArray);
                return result;
            }
        }

        return -1;
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



    static inline int parseNextValue(TokenizerParams_parent_ch_start_context, JsonArrayStruct* jsonArray)
    {
        int result;
        if (-1 < (result = Tokenizers::jsonValueTokenizer2(TokenizerParams_pass))) {
            auto *nextItem = Alloc::newJsonArrayItem(context, parent);

            nextItem->valueNode = context->virtualCodeNode;
            appendRootNode(jsonArray, nextItem);
            jsonArray->parsePhase = phase::EXPECT_COMMA;
            return result;
        }
        return -1;
    }


    int internal_JsonArrayTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *jsonArray = Cast::downcast<JsonArrayStruct *>(parent);

        if (ch == ']') {
            context->scanEnd = true;
            context->setCodeNode(&jsonArray->endBodyNode);
            return start + 1;
        }

        if (jsonArray->parsePhase == phase::EXPECT_VALUE) {
            return parseNextValue(TokenizerParams_pass, jsonArray);
        }

        auto *currentKeyValueItem = jsonArray->lastItem;

        if (jsonArray->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->setCodeNode(&currentKeyValueItem->follwingComma);
                jsonArray->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            }
            else if (context->afterLineBreak) {
                // comma is not required after a line break
                return parseNextValue(TokenizerParams_pass, jsonArray);
            }
            return -1;
        }
        return -1;
    }
}
