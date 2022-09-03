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
        EXPECT_NAME = 0,
        DELIMETER = 1,
        VALUE = 2,
        EXPECT_COMMA = 3
    };

    // -----------------------------------------------------------------------------------
    //
    //                              JsonKeyValueItemStruct
    //
    // -----------------------------------------------------------------------------------

    static CodeLine *appendToLine2(JsonKeyValueItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        if (self->keyNode) {
            currentCodeLine = VTableCall::appendToLine(self->keyNode, currentCodeLine);
        }

        currentCodeLine = VTableCall::appendToLine(&self->delimeter, currentCodeLine);
        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::appendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    }


    static const utf8byte *selfText_JsonKeyValueItemStruct(JsonKeyValueItemStruct *) {
        return "";
    }

    static int selfTextLength2(JsonKeyValueItemStruct *) {
        return 0;
    }

    static int applyFuncToDescendants(JsonKeyValueItemStruct *Node, ApplyFunc_params) {
        return 0;
    }

    static constexpr const char class_chars[] = "<JsonKeyValueItem>";

    static node_vtable _jsonObjectKeyValueStructVTable = CREATE_VTABLE(JsonKeyValueItemStruct,
                                                                             selfTextLength2,
                                                                             selfText_JsonKeyValueItemStruct,
                                                                             appendToLine2,
                                                                       applyFuncToDescendants,
                                                                             class_chars, NodeTypeId::JsonKeyValueItem);

    const struct node_vtable *VTables::JsonKeyValueItemVTable = &_jsonObjectKeyValueStructVTable;


    JsonKeyValueItemStruct *
    Alloc::newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<JsonKeyValueItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_jsonObjectKeyValueStructVTable);

        Init::initSymbolNode(&keyValueItem->delimeter, context, keyValueItem, ':');
        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;

        keyValueItem->keyNode = nullptr;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }


    // -----------------------------------------------------------------------------------
    //
    //                              JsonObjectKeyNodeStruct
    //
    // -----------------------------------------------------------------------------------
    static CodeLine *appendToLine3(JsonObjectKeyNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }


    static const utf8byte *selfText3(JsonObjectKeyNodeStruct *self) {
        return self->text;
    }

    static int selfTextLength3(JsonObjectKeyNodeStruct *self) {
        return self->textLength;
    }

    static int JsonObjectKeyNodeStruct_applyFuncToDescendants(JsonObjectKeyNodeStruct *node, ApplyFunc_params)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }

    static node_vtable _jsonObjectKeyStructVTable = CREATE_VTABLE(JsonObjectKeyNodeStruct,
                                                                        selfTextLength3, selfText3,
                                                                        appendToLine3,
                                                                  JsonObjectKeyNodeStruct_applyFuncToDescendants,
                                                                        "<JsonObjectKeyNodeStruct>"
                                                                        , NodeTypeId::JsonObjectKey);

    JsonObjectKeyNodeStruct *
    Alloc::newJsonObjectKeyNode(ParseContext *context, NodeBase *parentNode) {
        auto *jsonKey = context->newMem<JsonObjectKeyNodeStruct>();
        INIT_NODE(jsonKey, context, parentNode, &_jsonObjectKeyStructVTable);

        jsonKey->text = nullptr;
        jsonKey->nameLength = jsonKey->textLength = 0;
        jsonKey->namePos = 0;
        return jsonKey;
    }

    int Tokenizers::jsonObjectNameTokenizer(TokenizerParams_parent_ch_start_context) {
        int found_count = 0;

        // starts with "
        bool startsWithDQuote = false;
        if (context->chars[start] == '"') {
            startsWithDQuote = true;
            found_count++;
        }

        int letterStart = startsWithDQuote ? start + 1 : start;
        for (int_fast32_t i = letterStart; i < context->length; i++) {
            if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            } else if (startsWithDQuote) {
                found_count++;

                if (context->chars[i] == '"') {
                    break;
                }
            } else {
                break;
            }
        }

        if (found_count > 0) {
            auto *keyNode = Alloc::newJsonObjectKeyNode(context, parent);
            context->setCodeNode(keyNode);

            keyNode->namePos = 1;
            keyNode->nameLength = found_count - 2;
            {
                keyNode->text = context->memBuffer.newMem<char>(found_count + 1);
                keyNode->textLength = found_count;

                memcpy(keyNode->text, context->chars + start, found_count);
                keyNode->text[found_count] = '\0';
            }
            return start + found_count;
        }

        return -1;
    }


    const struct node_vtable *VTables::JsonObjectKeyVTable = &_jsonObjectKeyStructVTable;


    // ---------------------------------------------------------------------------------
    //
    //                              JsonObjectStruct
    //
    // ---------------------------------------------------------------------------------
    static int selfTextLength(JsonObjectStruct *) {
        return 1;
    }

    static const utf8byte *selfText(JsonObjectStruct *) {
        return "{";
    }

    static constexpr const char _typeName[] = "<JsonObject>";

    static CodeLine *appendToLine(JsonObjectStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;

        JsonKeyValueItemStruct *item = self->firstKeyValueItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<JsonKeyValueItemStruct *>(item->nextNode);
        }

        auto* prevCodeLine = currentCodeLine;
        currentCodeLine = VTableCall::appendToLine(&self->endBodyNode, currentCodeLine);
        if (prevCodeLine != currentCodeLine) {
            currentCodeLine->depth = formerParentDepth+1;
        }

        self->context->parentDepth = formerParentDepth;

        return currentCodeLine;
    }

    static int JsonObjectStruct_applyFuncToDescendants(JsonObjectStruct *node, ApplyFunc_params)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        JsonKeyValueItemStruct *item = node->firstKeyValueItem;
        while (item != nullptr) {
            item->vtable->applyFuncToDescendants(Cast::upcast(item), ApplyFunc_pass2);

            if (item->nextNode == nullptr) { break; }
            item = Cast::downcast<JsonKeyValueItemStruct *>(item->nextNode);
        }

        return 0;
    }

    static node_vtable _jsonObjectVTable = CREATE_VTABLE(JsonObjectStruct,
                                                               selfTextLength, selfText,
                                                               appendToLine, JsonObjectStruct_applyFuncToDescendants,  _typeName, NodeTypeId::JsonObject);
    const struct node_vtable *VTables::JsonObjectVTable = &_jsonObjectVTable;


    JsonObjectStruct *Alloc::newJsonObject(ParseContext *context, NodeBase *parentNode) {
        auto *jsonObjectNode = context->newMem<JsonObjectStruct>();
        INIT_NODE(jsonObjectNode, context, parentNode, VTables::JsonObjectVTable);
        jsonObjectNode->firstKeyValueItem = nullptr;
        jsonObjectNode->lastKeyValueItem = nullptr;
        jsonObjectNode->parsePhase = phase::EXPECT_NAME;

        jsonObjectNode->hashMap = context->newMem<VoidHashMap>();
        jsonObjectNode->hashMap->init(&context->memBuffer);

        Init::initSymbolNode(&jsonObjectNode->endBodyNode, context, parentNode, '}');

        return jsonObjectNode;
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

    void JsonUtils::put(JsonObjectStruct *json, utf8byte *key, int keyLength, NodeBase *node) {
        json->hashMap->put((const char *) key, keyLength, node);

        auto *newItem = Alloc::newJsonKeyValueItemNode(json->context, Cast::upcast(json));
        appendRootNode(json, newItem);
    }


    static int internal_JsonObjectTokenizerMulti(TokenizerParams_parent_ch_start_context);

    // --------------------- Implements JsonObject Parser ----------------------
    //  TODO: Add supports for new syntax like  @<MutableDict>{ awef:"fjiowe", test:true }
    int Tokenizers::jsonObjectTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '{') {
            int returnPosition = start + 1;
            auto *jsonObject = Alloc::newJsonObject(context, parent);
            int result = Scanner::scanMulti(jsonObject,
                                            internal_JsonObjectTokenizerMulti,
                                            returnPosition,
                                            context);

            if (result > -1) {
                context->setCodeNode(jsonObject);

                returnPosition = result;
                return returnPosition;
            }
        }

        return -1;
    }


    // object name
    // let$ val = {
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


    inline int parseObjectKey(TokenizerParams_parent_ch_start_context, JsonObjectStruct* jsonObject)
    {
        int result;
        if (-1 < (result = Tokenizers::jsonObjectNameTokenizer(parent, ch, start, context))) {
            JsonKeyValueItemStruct* nextItem = Alloc::newJsonKeyValueItemNode(context, parent);

            nextItem->keyNode = Cast::downcast<JsonObjectKeyNodeStruct*>(context->virtualCodeNode);

            if (jsonObject->firstKeyValueItem == nullptr) {
                jsonObject->firstKeyValueItem = nextItem;
            }
            else {
                jsonObject->lastKeyValueItem->nextNode = Cast::upcast(nextItem);
            }
            jsonObject->lastKeyValueItem = nextItem;

            jsonObject->parsePhase = DELIMETER;
            return result;
        }

        return -1;
    }


    int internal_JsonObjectTokenizerMulti(TokenizerParams_parent_ch_start_context)
    {
        auto *jsonObject = Cast::downcast<JsonObjectStruct *>(parent);

        if (jsonObject->parsePhase == phase::EXPECT_NAME) {
            if (ch == '}') { // empty
                context->scanEnd = true;
                context->setCodeNode(&jsonObject->endBodyNode);
                return start + 1;
            }

            return parseObjectKey(TokenizerParams_pass, jsonObject);
        }

        auto *currentKeyValueItem = jsonObject->lastKeyValueItem;
        assert(currentKeyValueItem != nullptr);

        if (jsonObject->parsePhase == phase::DELIMETER) {
            if (ch == ':') { // delimeter
                context->setCodeNode(&currentKeyValueItem->delimeter);
                jsonObject->parsePhase = phase::VALUE;
                return start + 1;
            }

            context->setError(ErrorCode::missing_object_delemeter, start);
            return SyntaxErrorInfo::SYNTAX_ERROR_RETURN;
        }


        if (jsonObject->parsePhase == phase::VALUE) {
            int result;
            if (-1 < (result = Tokenizers::jsonValueTokenizer2(Cast::upcast(currentKeyValueItem), ch,
                                                              start, context))) {
                currentKeyValueItem->valueNode = context->virtualCodeNode;
                jsonObject->parsePhase = phase::EXPECT_COMMA;
                return result;
            }
            return -1;
        }


        if (jsonObject->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->setCodeNode(&currentKeyValueItem->follwingComma);
                jsonObject->parsePhase = phase::EXPECT_NAME;
                return start + 1;
            } else if (ch == '}') {
                context->scanEnd = true;
                context->setCodeNode(&jsonObject->endBodyNode);
                return start + 1;
            }
            else if (context->afterLineBreak) {
                // comma is not needed after a Line break
                return parseObjectKey(TokenizerParams_pass, jsonObject);
            }
            return -1;
        }

        return -1;
    }



    int Tokenizers::jsonValueTokenizer2(TokenizerParams_parent_ch_start_context) {

        int result = Tokenizers::numberTokenizer(TokenizerParams_pass);
        if (result > -1) {
            return result;
        }

        if (-1 < (result = Tokenizers::boolTokenizer(TokenizerParams_pass))) {
            return result;
        }

        if (-1 < (result = Tokenizers::jsonObjectTokenizer(TokenizerParams_pass))) {
            return result;
        }

        if (-1 < (result = Tokenizers::jsonArrayTokenizer(TokenizerParams_pass))) {
            return result;
        }

        if (-1 < (result = Tokenizers::nullTokenizer(TokenizerParams_pass))) {
            return result;
        }

        if (-1 < (result = Tokenizers::stringLiteralTokenizer(TokenizerParams_pass))) {
            return result;
        }

        return -1;
    }
}
