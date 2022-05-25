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

    static constexpr const char fn_chars[] = "fn";
    static constexpr const char fn_first_char = fn_chars[0];
    static constexpr unsigned int size_of_fn = sizeof(fn_chars) - 1;



    /*
     *
     */

    static st_textlen selfTextLength2(BodyNodeStruct *classNode) {
        return 1;
    }

    static const utf8byte *selfText2(BodyNodeStruct *node) {
        return "{";
    }

    static CodeLine *appendToLine2(BodyNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;


        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(self);


        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;

        {
            auto *child = classNode->firstChildNode;
            while (child) {
                currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
                child = child->nextNode;
            }
        }


        auto *prevCodeLine = currentCodeLine;
        currentCodeLine = VTableCall::appendToLine(&classNode->endBodyNode, currentCodeLine);

        if (prevCodeLine != currentCodeLine) {
            currentCodeLine->depth = formerParentDepth + 1;
        }

        self->context->parentDepth = formerParentDepth;


        return currentCodeLine;
    };


    static constexpr const char bodyTypeText[] = "<body>";

/*
 * static low fn A<T>(a: int, b: String) {
 *
 *
 * }
 */
    static const node_vtable _BodyVTable = CREATE_VTABLE(BodyNodeStruct,
                                                       selfTextLength2,
                                                       selfText2,
                                                       appendToLine2,
                                                       bodyTypeText, NodeTypeId::Body);

    const struct node_vtable *VTables::BodyVTable = &_BodyVTable;

    void Init::initBodyNode(BodyNodeStruct *node, ParseContext *context, void *parentNode) {
        INIT_NODE(node, context, parentNode, VTables::BodyVTable);

        node->lastChildNode = nullptr;
        node->firstChildNode = nullptr;
        node->childCount = 0;
        node->startFound = false;

        Init::initSymbolNode(&node->bodyStartNode, context, node, '{');
        Init::initSymbolNode(&node->endBodyNode, context, node, '}');
    }


/*
    static void appendChildNode(BodyNodeStruct *body, NodeBase *node) {
        if (body->firstChildNode == nullptr) {
            body->firstChildNode = node;
        }
        if (body->lastChildNode != nullptr) {
            body->lastChildNode->nextNode = node;
        }
        body->lastChildNode = node;
        body->childCount++;
    }
 */

    static int inner_bodyTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *body = Cast::downcast<BodyNodeStruct *>(parent);
        if (ch == '}') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&body->endBodyNode);
            return start + 1;
        } else {
            //int result;
            /*
            if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                appendChildNode(body, context->codeNode);
                return result;
            }

            if (-1 < (result = Tokenizers::fnTokenizer(parent, ch, start, context))) {
                auto* innerClassNode = Cast::downcast<ClassNodeStruct*>(parent);
                appendChildNode(body, context->codeNode);
                return result;
            }
             */
        }

        return -1;
    }

    int Tokenizers::bodyTokenizer(TokenizerParams_parent_ch_start_context) {
        if (ch == '{') {
            int returnPosition = start + 1;

            auto *bodyNode = Cast::downcast<BodyNodeStruct *>(parent);

            int result = Scanner::scan(bodyNode,
                                       inner_bodyTokenizer,
                                       returnPosition,
                                       context);

            if (result > -1) {
                context->codeNode = Cast::upcast(bodyNode);

                returnPosition = result;
                return returnPosition;
            }
        }
        return -1;
    };



















// --------------------- Defines FuncNode VTable ---------------------- /

    static st_textlen selfTextLength(FuncNodeStruct *classNode) {
        return size_of_fn;
    }

    static const utf8byte *selfText(FuncNodeStruct *node) {
        return fn_chars;
    }

    static CodeLine *appendToLine(FuncNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;


        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(classNode);


        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;
        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
        self->context->parentDepth = formerParentDepth;




        currentCodeLine = VTableCall::appendToLine(&classNode->parameterStartNode, currentCodeLine);
        currentCodeLine = VTableCall::appendToLine(&classNode->parameterEndNode, currentCodeLine);

        currentCodeLine = VTableCall::appendToLine(&classNode->bodyNode, currentCodeLine);

        return currentCodeLine;
    };


    static constexpr const char fnTypeText[] = "<fn>";

/*
 * static low fn A<T>(a: int, b: String) {
 *
 *
 * }
 */
    static const node_vtable _FnVTable = CREATE_VTABLE(FuncNodeStruct,
                                                          selfTextLength,
                                                          selfText,
                                                          appendToLine,
                                                       fnTypeText, NodeTypeId::Func);

    const struct node_vtable *VTables::FnVTable = &_FnVTable;

    FuncNodeStruct* Alloc::newFuncNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *funcNode = context->newMem<FuncNodeStruct>();

        INIT_NODE(funcNode, context, parentNode, &_FnVTable);

        funcNode->lastChildParameterNode = nullptr;
        funcNode->firstChildParameterNode = nullptr;


        Init::initNameNode(&funcNode->nameNode, context, parentNode);

        funcNode->parameterStartFound = false;
        funcNode->parameterEndFound = false;

        Init::initSymbolNode(&funcNode->parameterStartNode, context, funcNode, '(');
        Init::initSymbolNode(&funcNode->parameterEndNode, context, funcNode, ')');

        Init::initBodyNode(&funcNode->bodyNode, context, funcNode);

        return funcNode;
    }


    // --------------------- Implements ClassNode Parser ----------------------
/*
    static void appendChildBodyNode(FuncNodeStruct *fnNode, NodeBase *node) {
        if (fnNode->firstChildBodyNode == nullptr) {
            fnNode->firstChildBodyNode = node;
        }
        if (fnNode->lastChildBodyNode != nullptr) {
            fnNode->lastChildBodyNode->nextNode = node;
        }
        fnNode->lastChildBodyNode = node;
        fnNode->childCount++;
    }
*/

/*
    int internal_parameterListTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *jsonArray = Cast::downcast<JsonArrayStruct *>(parent);

        if (ch == ']') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&jsonArray->endBodyNode);
            return start + 1;
        }

        if (jsonArray->parsePhase == phase::EXPECT_VALUE) {
            return parseNextValue(TokenizerParams_pass, jsonArray);
        }

        auto *currentKeyValueItem = jsonArray->lastItem;

        if (jsonArray->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->codeNode = Cast::upcast(&currentKeyValueItem->follwingComma);
                jsonArray->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            } else if (context->afterLineBreak) {
                // comma is not needed after a line break
                return parseNextValue(TokenizerParams_pass, jsonArray);
            }
            return -1;
        }

        return -1;
    }
 */

    static int fnBodyTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *fnNode = Cast::downcast<FuncNodeStruct *>(parent);

        //console_log(std::string(""+ch).c_str());
        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());

        if (!fnNode->parameterStartFound) {
            if (ch == '(') {
                fnNode->parameterStartFound = true;
                context->codeNode = Cast::upcast(&fnNode->parameterStartNode);
                return start + 1;
            }
        } else {
            if (!fnNode->parameterEndFound) {
                if (ch == ')') {
                    fnNode->parameterEndFound = true;
                    context->codeNode = Cast::upcast(&fnNode->parameterEndNode);
                    return start + 1;
                } else {
                    /*
                        int result;
                        if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                            auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                            appendChildNode(innerClassNode, context->codeNode);
                            return result;
                        }
                         */

                }
            } else {
                int result;
                if (-1 < (result = Tokenizers::bodyTokenizer(Cast::upcast(&fnNode->bodyNode), ch, start, context))) {
                    return result;
                }
            }
        }
        return -1;
    };


    int Tokenizers::fnTokenizer(TokenizerParams_parent_ch_start_context) {


        if (fn_first_char == ch) {
            auto idx = ParseUtil::matchFirstWithTrim(context->chars, fn_chars, start);
            if (idx > -1) {
                if (idx + size_of_fn < context->length
                    && ParseUtil::isSpaceOrLineBreak(context->chars[idx + size_of_fn])
                        ) {

                    int currentPos = idx + size_of_fn;
                    int resultPos = -1;

                    // console_log(std::string("wow5").c_str());

                    // "fn " came here
                    auto *fnNode = Alloc::newFuncNode(context, parent);

                    {
                        resultPos = Scanner::scan(&fnNode->nameNode,
                                                  Tokenizers::nameTokenizer,
                                                  currentPos,
                                                  context);

                        if (resultPos == -1) {
                            // the class should have a class name
                            //console_log(std::string(classNode->nameNode.name).c_str());

                            context->codeNode = Cast::upcast(fnNode);
                            return currentPos;
                        }

                        //console_log("name=" + std::string(classNode->nameNode.name));
                    }


                    // Parse body
                    currentPos = resultPos;
                    if (-1 == (resultPos = Scanner::scan(fnNode, fnBodyTokenizer,
                                                         currentPos, context))) {
                        context->codeNode = Cast::upcast(fnNode);
                        return currentPos;
                    }

                    context->codeNode = Cast::upcast(fnNode);
                    return resultPos;
                }
            }
        }
        return -1;
    };

/*
int FuncTokenizer::tryTokenize(CodeNodeBase *parentNode,utf8byte first_ch,int i, ParseContext *context) {
    if ('f' == first_ch) {
        auto idx = Tokenizer::matchFirstWithTrim(context->chars, fn_chars, i);
        if (idx > -1) {
            if (Tokenizer::isSpace(context->chars[idx + fn_chars_length])) {
                int returnPosition = idx + fn_chars_length;

                // "class " came, generate ClassNode
                funcNode = new FuncNode(parentNode);
                context->codeNode = funcNode;

                {
                    // Tokenize class Name, class AClass
                    NameNode<FuncNode>::NameTokenizer nameTokenizer;
                    int result = scanChars(funcNode, nameTokenizer, returnPosition, context, false);
                    if (result == -1) {
                        context->codeNode = funcNode;
                        return returnPosition;
                    }
                    funcNode->funcName = nameTokenizer.nameNode;
                    returnPosition = result;
                }


                {
                    //  Tokenize body  {}
                    FuncBodyTokenizer bodyTokenizer;
                    int result = scanChars(funcNode, bodyTokenizer, returnPosition, context, false);
                    if (result == -1) {
                        context->codeNode = funcNode;
                        return returnPosition;
                    }

                    funcNode->bodyNode = bodyTokenizer.bodyNode;
                    returnPosition = result;
                }

                context->codeNode = funcNode;
                return returnPosition;
            }
        }
    }

    return -1;
};

CodeLine *FuncNode::appendToLine(CodeLine *currentCodeLine) {
    currentCodeLine = addLineBreakNode(currentCodeLine);
    currentCodeLine->nodes.push_back(this);
    if (funcName) {
        currentCodeLine = funcName->appendToLine(currentCodeLine);
    }
    if (bodyNode) {
        currentCodeLine = bodyNode->appendToLine(currentCodeLine);
    }
    return currentCodeLine;

}
*/
}