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
        EXPECT_NAME = 0,
        DELIMETER = 1,
        VALUE = 2,
        COMMA = 3
    };

    // --------------------- Defines JsonKeyValueItemStruct VTable ---------------------- /

    static CodeLine *appendToLine2(JsonKeyValueItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        currentCodeLine = VTableCall::appendToLine(&self->keyNode, currentCodeLine);
        currentCodeLine = VTableCall::appendToLine(&self->delimeter, currentCodeLine);
        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::appendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    };


    static const utf8byte *selfText_JsonKeyValueItemStruct(JsonKeyValueItemStruct *self) {
        return "";//self->keyNode.name;// "b:9";
    }

    static int selfTextLength2(JsonKeyValueItemStruct *self) {
        return 0;// self->keyNode.nameLength;// 3;
    }


    static constexpr const char class_chars[] = "<JsonKeyValueItem>";
    static const utf8byte *typeText2(JsonKeyValueItemStruct *self) {
        return  class_chars;
    }

    static int typeTextLength2(JsonKeyValueItemStruct *self) {
        return sizeof(class_chars) - 1;
        //const char foo[] = "<JsonKeyValueItem>";
        //size_t len = sizeof(foo) - 1;
        //return len;
    }

    static const node_vtable _JsonObjectKeyValueStructVTable = CREATE_VTABLE(JsonKeyValueItemStruct,
                                                                             selfTextLength2,
                                                                             selfText_JsonKeyValueItemStruct,
                                                                             appendToLine2, typeTextLength2,                                                        typeText2);

    const struct node_vtable *VTables::JsonKeyValueItemVTable = &_JsonObjectKeyValueStructVTable;








    // --------------------- Defines JsonObjectStruct VTable ---------------------- /
    static int selfTextLength(JsonObjectStruct *self) {
        return 1;
    }

    static const utf8byte *selfText(JsonObjectStruct *node) {
        return "{";
    }


    static constexpr const char _typeName[] = "<JsonObject>";
    static int typeTextLength(JsonObjectStruct *self) {
        return sizeof(_typeName) - 1;
    }

    static const utf8byte *typeText(JsonObjectStruct *node) {
        return _typeName;
    }

    static CodeLine *appendToLine(JsonObjectStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);


        JsonKeyValueItemStruct *item = self->firstKeyValueItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<JsonKeyValueItemStruct*>(item->nextNode);
        }

        currentCodeLine = VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
        return currentCodeLine;
    };



    static const node_vtable _JsonObjectVTable = CREATE_VTABLE(JsonObjectStruct,
                                                               selfTextLength, selfText,
                                                               appendToLine, typeTextLength, typeText);
    const struct node_vtable *VTables::JsonObjectVTable = &_JsonObjectVTable;












    // -------------------- Implements ClassNode Allocator --------------------- //

    JsonObjectStruct *Alloc::newJsonObject(ParseContext *context, NodeBase *parentNode) {
        auto *jsonObjectNode = simpleMalloc<JsonObjectStruct>();
        INIT_NODE(jsonObjectNode, context, parentNode, VTables::JsonObjectVTable);
        jsonObjectNode->firstKeyValueItem = nullptr;
        jsonObjectNode->lastKeyValueItem = nullptr;
        jsonObjectNode->parsePhase = phase::EXPECT_NAME;

        INIT_NODE(&jsonObjectNode->endBodyNode,
                  context,
                  Cast::upcast(jsonObjectNode),
                  VTables::SymbolVTable);
        jsonObjectNode->endBodyNode.symbol[0] = '}';
        jsonObjectNode->endBodyNode.symbol[1] = '\0';

        return jsonObjectNode;
    }


    void Alloc::deleteJsonObject(NodeBase *node) {
        auto *classNode = Cast::downcast<JsonObjectStruct *>(node);

        //if (classNode->nameNode.name != nullptr) {
        //free(classNode->nameNode.name);
        //classNode->nameNode.name = nullptr;
        //}

        free(classNode);
    }

    int Tokenizers::jsonObjectNameTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;
        
        // starts with "
        bool startWithDQuote = false;
        if (context->chars[start] == '"') {
            startWithDQuote = true;
            found_count++;
        }

        int letterStart = startWithDQuote ? start + 1 : start;
        for (uint_fast32_t i = letterStart; i < context->length; i++) {
            if (Tokenizer::isIdentifierLetter(context->chars[i])) {
                found_count++;
            }
            else if (startWithDQuote) {
                found_count++;

                if (context->chars[i] == '"') {
                    break;
                }
            } else {
                break;
            }
        }

        if (found_count > 0) {
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            context->codeNode = Cast::upcast(nameNode);
            nameNode->name = context->charBuffer.newChars(found_count + 1);
            nameNode->nameLength = found_count;

            memcpy(nameNode->name, context->chars + start, found_count);
            nameNode->name[found_count] = '\0';

            return start + found_count;
        }

        return -1;
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



    static int internal_JsonObjectTokenizer(TokenizerParams_parent_ch_start_context);

    // --------------------- Implements JsonObject Parser ----------------------
    //  TODO: Add supports for new syntax like  @<MutableDict>{ awef:"fjiowe", test:true }
    int Tokenizers::jsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '{') {
            int returnPosition = start + 1;
            auto *jsonObject = Alloc::newJsonObject(context, parent);
            int result = Scanner::scan(jsonObject,
                                       internal_JsonObjectTokenizer,
                                       returnPosition,
                                       context);

            if (result > -1) {
                context->codeNode = Cast::upcast(jsonObject);

                returnPosition = result;
                return returnPosition;

            }
        }

        return -1;
    }









    JsonKeyValueItemStruct *Alloc::newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = simpleMalloc<JsonKeyValueItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_JsonObjectKeyValueStructVTable)

        Init::initNameNode(&keyValueItem->keyNode, context, parentNode);
        Init::initSymbolNode(&keyValueItem->delimeter, context, keyValueItem, ':');
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


    int internal_JsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {

        auto *jsonObject = Cast::downcast<JsonObjectStruct *>(parent);

        if (jsonObject->parsePhase == phase::EXPECT_NAME) {
            if (ch == '}') {
                context->scanEnd = true;
                context->codeNode = Cast::upcast(&jsonObject->endBodyNode);
                return start + 1;
            }

            JsonKeyValueItemStruct *nextItem = Alloc::newJsonKeyValueItemNode(context, parent);

            int result;
            if (-1 < (result = Tokenizers::jsonObjectNameTokenizer(Cast::upcast(&nextItem->keyNode), ch, start, context))) {

                if (jsonObject->firstKeyValueItem == nullptr) {
                    jsonObject->firstKeyValueItem = nextItem;
                }
                else {
                    jsonObject->lastKeyValueItem->nextNode = Cast::upcast(nextItem);
                }
                jsonObject->lastKeyValueItem = nextItem;

                jsonObject->parsePhase = phase::DELIMETER;
                return result;
            }
            return -1;
        }

        auto *currentKeyValueItem = jsonObject->lastKeyValueItem;
        if (jsonObject->parsePhase == phase::DELIMETER) {
            if (ch == ':') { // delimeter
                context->codeNode = Cast::upcast(&currentKeyValueItem->delimeter);
                jsonObject->parsePhase = phase::VALUE;
                return start + 1;
            }
            return -1;
        }


        if (jsonObject->parsePhase == phase::VALUE) {
            int result;
            if (-1 < (result = Tokenizers::jsonValueTokenizer(Cast::upcast(currentKeyValueItem), ch, start, context))) {
                currentKeyValueItem->valueNode = context->codeNode;
                jsonObject->parsePhase = phase::COMMA;
                context->scanEnd = false;
                return result;
            }
            return -1;
        }


        if (jsonObject->parsePhase == phase::COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->codeNode = Cast::upcast(&currentKeyValueItem->follwingComma);
                jsonObject->parsePhase = phase::EXPECT_NAME;
                return start + 1;
            }
            else if (ch == '}') {
                context->scanEnd = true;
                context->codeNode = Cast::upcast(&jsonObject->endBodyNode);
                return start + 1;
            }
            return -1;
        }

        return -1;
    }


    int Tokenizers::jsonValueTokenizer(TokenizerParams_parent_ch_start_context) {
        int result = Tokenizers::numberTokenizer(TokenizerParams_pass);
        if (result > -1) {
            return result;
        }

        result = Tokenizers::boolTokenizer(TokenizerParams_pass);
        if (result > -1) {
            return result;
        }

        if (-1 < (result = Tokenizers::jsonObjectTokenizer(TokenizerParams_pass))) {
            return result;
        }

        return -1;
    }

}
