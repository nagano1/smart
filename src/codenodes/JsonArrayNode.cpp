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

    static enum phase {
        EXPECT_VALUE = 0,
        COMMA = 3
    };






    // --------------------- Defines JsonKeyValueItemStruct VTable ---------------------- /

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
        return "";//self->keyNode.name;// "b:9";
    }

    static int selfTextLength2(JsonArrayItemStruct *self) {
        return 0;// self->keyNode.nameLength;// 3;
    }


    static constexpr const char class_chars2[] = "<JsonArrayItem>";

    static const node_vtable _JsonArrayItemStructVTable = CREATE_VTABLE(JsonArrayItemStruct,
        selfTextLength2,
        selfText_JsonKeyValueItemStruct,
        appendToLine2, class_chars2);

    const struct node_vtable *VTables::JsonArrayItemVTable = &_JsonArrayItemStructVTable;

















    // --------------------- Defines JsonObjectStruct VTable ---------------------- /
    static int selfTextLength(JsonArrayStruct *self) {
        return 1;
    }

    static const utf8byte *selfText(JsonArrayStruct *node) {
        return "[";
    }

    static constexpr const char _typeName[] = "<JsonArray>";

    static CodeLine *appendToLine(JsonArrayStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);


        JsonArrayItemStruct *item = self->firstItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<JsonArrayItemStruct*>(item->nextNode);
        }

        currentCodeLine = VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
        return currentCodeLine;
    };



    static const node_vtable _VTable = CREATE_VTABLE(JsonArrayStruct,
                                                               selfTextLength, selfText,
                                                               appendToLine, _typeName);
    const struct node_vtable *VTables::JsonArrayVTable = &_VTable;












    // -------------------- Implements JsonObjectStruct --------------------- //

    JsonArrayStruct *Alloc::newJsonArray(ParseContext *context, NodeBase *parentNode) {
        auto *jsonObjectNode = simpleMalloc<JsonArrayStruct>();
        INIT_NODE(jsonObjectNode, context, parentNode, VTables::JsonArrayVTable);
        jsonObjectNode->firstItem= nullptr;
        jsonObjectNode->lastItem = nullptr;
        jsonObjectNode->parsePhase = phase::EXPECT_VALUE;

        INIT_NODE(&jsonObjectNode->endBodyNode,
                  context,
                  Cast::upcast(jsonObjectNode),
                  VTables::SymbolVTable);
        jsonObjectNode->endBodyNode.symbol[0] = ']';
        jsonObjectNode->endBodyNode.symbol[1] = '\0';

        return jsonObjectNode;
    }


    void Alloc::deleteJsonArray(NodeBase *node) {
        auto *classNode = Cast::downcast<JsonArrayStruct*>(node);

        //if (classNode->nameNode.name != nullptr) {
        //free(classNode->nameNode.name);
        //classNode->nameNode.name = nullptr;
        //}

        free(classNode);
    }


    static void appendRootNode(JsonObjectStruct *doc, JsonKeyValueItemStruct *node) {
        if (doc->firstKeyValueItem == nullptr) {
            doc->firstKeyValueItem = node;
        }
        if (doc->lastKeyValueItem != nullptr) {
            doc->lastKeyValueItem->nextNode = (NodeBase *) node;
        }
        doc->lastKeyValueItem = node;
    }



    static int internal_JsonArrayTokenizer(TokenizerParams_parent_ch_start_context);

    // --------------------- Implements JsonObject Parser ----------------------
    //  TODO: Add supports for new syntax like  @<MutableDict>{ awef:"fjiowe", test:true }
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









    JsonArrayItemStruct *Alloc::newJsonArrayItem (ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = simpleMalloc<JsonArrayItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_JsonArrayItemStructVTable)

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }

        // object name
        // var val = {
        //   name : "valuevar"
        //   v2: true
        //   watashi: (234 + 512
        //             - 512 * 2)
        //   gauge: true || false
        //   var32324: awfe.fwfw()
        //                 .awfe()
        // }
        // aaawef = awe.fwe()
        //             .func()
        //             + 234123
        //             + 1234
        // (-1243).afwef; test();
        // aweff = 2342


    int internal_JsonArrayTokenizer(TokenizerParams_parent_ch_start_context) {

        auto *jsonObject = Cast::downcast<JsonArrayStruct *>(parent);

        if (jsonObject->parsePhase == phase::EXPECT_VALUE) {
            if (ch == ']') {
                context->scanEnd = true;
                context->codeNode = Cast::upcast(&jsonObject->endBodyNode);
                return start + 1;
            }

            int result;
            if (-1 < (result = Tokenizers::jsonValueTokenizer(parent, ch, start, context))) {
                auto *nextItem = Alloc::newJsonArrayItem(context, parent);

                nextItem->valueNode = context->codeNode;

                if (jsonObject->firstItem == nullptr) {
                    jsonObject->firstItem = nextItem;
                }
                else {
                    jsonObject->lastItem ->nextNode = Cast::upcast(nextItem);
                }
                jsonObject->lastItem = nextItem;




                jsonObject->parsePhase = phase::COMMA;
                context->scanEnd = false;
                return result;
            }
            return -1;
        }

        auto *currentKeyValueItem = jsonObject->lastItem;

        if (jsonObject->parsePhase == phase::COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->codeNode = Cast::upcast(&currentKeyValueItem->follwingComma);
                jsonObject->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            }
            else if (ch == ']') {
                context->scanEnd = true;
                context->codeNode = Cast::upcast(&jsonObject->endBodyNode);
                return start + 1;
            }
            return -1;
        }

        return -1;
    }

}
