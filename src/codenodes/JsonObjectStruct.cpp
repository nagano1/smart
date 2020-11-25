﻿#include <stdio.h>
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

    /**
     * JsonKeyValueItemStruct
     *
     */

    // --------------------- Defines JsonKeyValueItemStruct VTable ---------------------- /

    static CodeLine *appendToLine2(JsonKeyValueItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        //currentCodeLine = VTableCall::appendToLine(&jsonObjectNode->endBodyNode, currentCodeLine);
        //currentCodeLine = VTableCall::appendToLine(&classNode->bodyNode, currentCodeLine);

        return currentCodeLine;
    };


    static const utf8byte *selfText_JsonKeyValueItemStruct(JsonKeyValueItemStruct *self) {
        return self->keyNode.name;// "b:9";
    }

    static int selfTextLength2(JsonKeyValueItemStruct *self) {
        return self->keyNode.nameLength;// 3;
    }

    static const node_vtable _JsonObjectKeyValueStructVTable = CREATE_VTABLE(JsonKeyValueItemStruct,
                                                                             selfTextLength2,
                                                                             selfText_JsonKeyValueItemStruct,
                                                                             appendToLine2);

    const struct node_vtable *VTables::JsonKeyValueItemVTable = &_JsonObjectKeyValueStructVTable;


    // --------------------- Defines JsonObjectStruct VTable ---------------------- /
    static int selfTextLength(JsonObjectStruct *self) {
        return 1;
    }

    static CodeLine *appendToLine(JsonObjectStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        currentCodeLine = VTableCall::appendToLine(self->firstKeyValueItem,
                                                   currentCodeLine);

        currentCodeLine = VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
        //currentCodeLine = VTableCall::appendToLine(&classNode->bodyNode, currentCodeLine);

        return currentCodeLine;
    };


    static const utf8byte *selfText(JsonObjectStruct *node) {
        return "{";
    }


    static const node_vtable _JsonObjectVTable = CREATE_VTABLE(JsonObjectStruct,
                                                               selfTextLength,
                                                               selfText,
                                                               appendToLine);
    const struct node_vtable *VTables::JsonObjectVTable = &_JsonObjectVTable;












    // -------------------- Implements ClassNode Allocator --------------------- //

    JsonObjectStruct *Allocator::newJsonObject(ParseContext *context, NodeBase *parentNode) {
        auto *classNode = simpleMalloc<JsonObjectStruct>();
        INIT_NODE(classNode, context, parentNode, &_JsonObjectVTable);
        classNode->firstKeyValueItem = nullptr;
        classNode->lastKeyValueItem = nullptr;
        classNode->parsePhase = 0;

        //Init::initNameNode(&classNode->nameNode, context);

        INIT_NODE(&classNode->endBodyNode,
                  context,
                  Cast::upcast(classNode),
                  VTables::SymbolVTable);
        classNode->endBodyNode.symbol[0] = '}';
        classNode->endBodyNode.symbol[1] = '\0';

        return classNode;
    }

    void Allocator::deleteJsonObject(NodeBase *node) {
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
            } else if (startWithDQuote && context->chars[i] == '"') {
                found_count++;
                break;
            } else {
                break;
            }
        }

        if (found_count > 0) {
            //context->scanEnd = true;
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            context->codeNode = Cast::upcast(nameNode);
            nameNode->name = context->charBuffer.newChars(found_count + 1);
            nameNode->nameLength = found_count;

            memcpy(nameNode->name, context->chars + start, found_count);
            printf("\nname = %s", nameNode->name);
            //nameNode->name[found_count] = '\0';
            return start + found_count;
        }
        return -1;
    }


    int Tokenizers::jsonValueTokenizer(TokenizerParams_parent_ch_start_context) {
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
        //doc->itemCount++;
    }


    JsonKeyValueItemStruct *
    Allocator::newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = simpleMalloc<JsonKeyValueItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_JsonObjectKeyValueStructVTable)

        Init::initNameNode(&keyValueItem->keyNode, context, parentNode);
        //keyValueItem->startFound = false;
        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        return keyValueItem;
    }

    static int internal_JsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *jsonObject = Cast::downcast<JsonObjectStruct *>(parent);
        printf("\njsonObject->parsePhase = %d\n", jsonObject->parsePhase);
        printf("\ncontext->afterLineBreak= %d\n", context->afterLineBreak);

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
        //             .func() + 234123
        //          + 1234
        //          ;
        // aweff = 2342;

        if (jsonObject->parsePhase == 0) {
            JsonKeyValueItemStruct *nextItem = Allocator::newJsonKeyValueItemNode(context, parent);
            if (jsonObject->firstKeyValueItem == nullptr) {
                jsonObject->firstKeyValueItem = nextItem;
            } else {
                jsonObject->lastKeyValueItem->nextNode = Cast::upcast(nextItem);
            }
            jsonObject->lastKeyValueItem = nextItem;

            int result;
            if (-1 < (result = Tokenizers::jsonObjectNameTokenizer(parent, ch, start, context))) {
                jsonObject->parsePhase = 1;
                return result;
            }
            return -1;
        }

        auto *lastKeyValueItem = jsonObject->lastKeyValueItem;
        if (jsonObject->parsePhase == 1) {
            if (ch == ':') { // delimeter
                context->codeNode = Cast::upcast(&lastKeyValueItem->delimeter);
                jsonObject->parsePhase = 2;
                return start + 1;
            }
            return -1;
        }

        if (jsonObject->parsePhase == 2) {
            int result;
            if (-1 < (result = Tokenizers::jsonObjectNameTokenizer(parent, ch, start, context))) {
                jsonObject->parsePhase = 3;
                return result;
            }
            return -1;
        }

        if (jsonObject->parsePhase == 3) {


        }



        // :

        //if (-1 < (result = Tokenizers::jsonValueTokenizer(parent, ch, start, context))) {
        //appendRootNode(doc, context->codeNode);
        //return result;
        //} else
 if (ch == ',') { // try to find ',' which leads to next key-value

        } else if (ch == '}') { // try to find '}' which finalizes this object

        }


        return -1;
    }

    // --------------------- Implements JsonObject Parser ----------------------
    //  TODO: Add supports for new syntax like  @<MutableDict>{ awef:"fjiowe", test:true }
    int Tokenizers::jsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '{') {
            int returnPosition = start + 1;
            auto *jsonObject = Allocator::newJsonObject(context, parent);
            int result = Scanner::scan(jsonObject,
                                       internal_JsonObjectTokenizer,
                                       returnPosition,
                                       context);
            printf("\n WWWWWWW");
            if (result > -1) {
                returnPosition = result;
            }

            context->codeNode = Cast::upcast(jsonObject);
            return returnPosition;
        }

        return -1;
    };
}
