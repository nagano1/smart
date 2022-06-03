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

    // --------------------- Defines AssignStatement VTable ---------------------- /

    static st_textlen selfTextLength(AssignStatementNodeStruct *) {
        return 3;
    }

    static const utf8byte *selfText(AssignStatementNodeStruct *self) {
        return self->useMut ? "mut" : "let";
    }

    static CodeLine *appendToLine(AssignStatementNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);
        currentCodeLine->appendNode(classNode);

//
//        auto formerParentDepth = self->context->parentDepth;
//        self->context->parentDepth += 1;
//        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
//        self->context->parentDepth = formerParentDepth;
//
//        currentCodeLine = VTableCall::appendToLine(&classNode->bodyStartNode, currentCodeLine);
//
//        formerParentDepth = self->context->parentDepth;
//        self->context->parentDepth += 1;
//
//        {
//            auto *child = classNode->firstChildNode;
//            while (child) {
//                currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
//                child = child->nextNode;
//            }
//        }
//
//
//
//        auto* prevCodeLine = currentCodeLine;
//        currentCodeLine = VTableCall::appendToLine(&classNode->endBodyNode, currentCodeLine);
//
//        if (prevCodeLine != currentCodeLine) {
//            currentCodeLine->depth = formerParentDepth+1;
//        }
//
//        self->context->parentDepth = formerParentDepth;


        return currentCodeLine;
    };


    static constexpr const char assignTypeText[] = "<AssignStatement>";

    /*
     * class A {
     *
     *
     * }
     */
    static const node_vtable _assignVTable = CREATE_VTABLE(AssignStatementNodeStruct,
                                                          selfTextLength,
                                                          selfText,
                                                          appendToLine,
                                                          assignTypeText
                                                          , NodeTypeId::AssignStatement);

    const struct node_vtable *const VTables::AssignStatementVTable = &_assignVTable;


    // -------------------- Implements AssignStatement Allocator --------------------- //
    AssignStatementNodeStruct *Alloc::newAssignStatement(ParseContext *context, NodeBase *parentNode) {
        auto *assignStatement = context->newMem<AssignStatementNodeStruct>();

        INIT_NODE(assignStatement, context, parentNode, &_assignVTable);


        assignStatement->useMut = false;
        assignStatement->valueNode = nullptr;
        assignStatement->startFound = false;


        Init::initNameNode(&assignStatement->nameNode, context, assignStatement);
        Init::initSymbolNode(&assignStatement->equalSymbol, context, assignStatement, '=');
        Init::initSimpleTextNode(&assignStatement->letOrMut, context, assignStatement, 3);

        return assignStatement;
    }


    // --------------------- Implements ClassNode Parser ----------------------
    static int inner_classBodyTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *classNode = Cast::downcast<ClassNodeStruct *>(parent);

        //console_log(std::string(""+ch).c_str());
        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());


        if (!classNode->startFound) {
            if (ch == '{') {
                classNode->startFound = true;
                context->codeNode = Cast::upcast(&classNode->bodyStartNode);
                return start + 1;
            }
        } else if (ch == '}') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&classNode->endBodyNode);
            return start + 1;
        } else {
            int result;
            if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                // auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                return result;
            }

            if (-1 < (result = Tokenizers::fnTokenizer(parent, ch, start, context))) {
                // auto* innerClassNode = Cast::downcast<ClassNodeStruct*>(parent);
                return result;
            }
        }

        return -1;
    }


    int Tokenizers::assignStatementTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char let_chars[] = "let";
        static constexpr unsigned int size_of_let = sizeof(let_chars) - 1;

        //static constexpr const char mut_chars[] = "mut";
        //static constexpr unsigned int size_of_mut = sizeof(mut_chars) - 1;


        if ('l' == ch || 'm' == ch) { //
            auto idx = ParseUtil::matchFirstWithTrim(context->chars, let_chars, start);
            if (idx > -1) {
                if (idx + size_of_let < context->length
                    && ParseUtil::isSpaceOrLineBreak(context->chars[idx + size_of_let])
                ) {

                    int currentPos = idx + size_of_let;
                    int resultPos;

                    // "class " came here
                    auto *assignStatement = Alloc::newAssignStatement(context, parent);

                    {
                        resultPos = Scanner::scanOnce(&assignStatement->nameNode,
                                                  Tokenizers::nameTokenizer,
                                                  currentPos,
                                                  context);

                        if (resultPos == -1) {
                            // the class should have a class name
                            //console_log(std::string(assignStatement->nameNode.name).c_str());

                            context->codeNode = Cast::upcast(assignStatement);
                            return currentPos;
                        }
                    }


                    // Parse body
                    currentPos = resultPos;
                    if (-1 == (resultPos = Scanner::scanMulti(assignStatement, inner_classBodyTokenizerMulti,
                                                              currentPos, context))) {
                        context->codeNode = Cast::upcast(assignStatement);
                        return currentPos;
                    }

                    context->codeNode = Cast::upcast(assignStatement);
                    return resultPos;
                }
            }
        }
        return -1;
    }
}



