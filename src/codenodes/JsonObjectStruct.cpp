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


    // --------------------- Defines JsonKeyValueItemStruct VTable ---------------------- /

    static int selfTextLength2(JsonKeyValueItemStruct *classNode) {
        return 3;
    }

    static CodeLine *
    appendToLine2(JsonKeyValueItemStruct *jsonObjectNode, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(jsonObjectNode);

        currentCodeLine->appendNode(jsonObjectNode);

        //currentCodeLine = VTableCall::appendToLine(&jsonObjectNode->endBodyNode, currentCodeLine);
        //currentCodeLine = VTableCall::appendToLine(&classNode->bodyNode, currentCodeLine);

        return currentCodeLine;
    };


    static const utf8byte *selfText2(JsonKeyValueItemStruct *node) {
        return "b:9";
    };


    static const node_vtable _JsonObjectKeyValueStructVTable = CREATE_VTABLE(JsonKeyValueItemStruct,
                                                                             selfTextLength2,
                                                                             selfText2,
                                                                             appendToLine2);

    const struct node_vtable *VTables::JsonKeyValueItemVTable = &_JsonObjectKeyValueStructVTable;




    // --------------------- Defines JsonObjectStruct VTable ---------------------- /

    static int selfTextLength(JsonObjectStruct *classNode) {
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
    };


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
        for (uint_fast32_t i = start; i < context->length; i++) {
            if (Tokenizer::isIdentifierLetter(context->chars[i])) {
                found_count++;
            } else {
                break;
            }
        }

        if (found_count > 0) {
            context->scanEnd = true;
            //auto *nameNode = Cast::downcast<NameNodeStruct *>(context->codeNode);
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            //auto *nameNode = Allocator::newNameNode(context, parent);
            context->codeNode = Cast::upcast(nameNode);
            nameNode->name = context->charBuffer.newChars(found_count + 1);
            // (char*)malloc(found_count+1);
            nameNode->nameLength = found_count;

            memcpy(nameNode->name, context->chars + start, found_count);
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


    static inline int internal_JsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *jsonObject = Cast::downcast<JsonObjectStruct *>(parent);
        JsonKeyValueItemStruct *currentKeyValueItem = nullptr;

        JsonKeyValueItemStruct *nextItem = Allocator::newJsonKeyValueItemNode(context, parent);


        jsonObject->firstKeyValueItem = nextItem;
        int result;
        if (-1 < (result = Tokenizers::jsonObjectNameTokenizer(parent, ch, start, context))) {
            return result;
        }

        if (-1 < (result = Tokenizers::jsonValueTokenizer(parent, ch, start, context))) {
            //appendRootNode(doc, context->codeNode);
            return result;
        } else if (ch == ':') { // delimeter

        } else if (ch == ',') { // try to find ',' which leads to next key-value

        } else if (ch == '}') { // try to find '}' which finalizes this object

        }

        return -1;
    }

    // --------------------- Implements JsonObject Parser ----------------------

    int Tokenizers::jsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '{') {
            int returnPosition = start + 1;
            auto *jsonObject = Allocator::newJsonObject(context, parent);

            int result = Scanner::scan(jsonObject,
                                       internal_JsonObjectTokenizer,
                                       returnPosition,
                                       context);
            if (result > -1) {
                returnPosition = result;
            }

            context->codeNode = Cast::upcast(jsonObject);
            return returnPosition;
        }

        return -1;
    };
}
