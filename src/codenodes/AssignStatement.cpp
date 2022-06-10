﻿#include <cstdio>
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

        if (self->useMut || self->useLet) {
            currentCodeLine = VTableCall::appendToLine(&self->letOrMut, currentCodeLine);
        }

        currentCodeLine = VTableCall::appendToLine(&self->nameNode, currentCodeLine);

        if (self->equalSymbol.found) {
            currentCodeLine = VTableCall::appendToLine(&self->equalSymbol, currentCodeLine);

            if (self->valueNode) {
                currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
            }
        }

        return currentCodeLine;
    }


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
        assignStatement->useLet = false;
        assignStatement->valueNode = nullptr;

        Init::initNameNode(&assignStatement->nameNode, context, assignStatement);
        Init::initSymbolNode(&assignStatement->equalSymbol, context, assignStatement, '=');
        Init::initSimpleTextNode(&assignStatement->letOrMut, context, assignStatement, 3);

        return assignStatement;
    }


    // --------------------- Implements ClassNode Parser ----------------------
    static int inner_assignStatementTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *assignment = Cast::downcast<AssignStatementNodeStruct *>(parent);

        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());
        if (!assignment->nameNode.found) {
            int result;
            if (-1 < (result = Tokenizers::nameTokenizer(Cast::upcast(&assignment->nameNode)
                                                        , ch, start, context))
            ) {
                context->codeNode = Cast::upcast(&assignment->nameNode);
                return result;
            }
        } else if (!assignment->equalSymbol.found) {
            if (ch == '=') {
                assignment->equalSymbol.found = true;
                context->codeNode = Cast::upcast(&assignment->equalSymbol);
                return start+1;
            } else {
                if (assignment->useMut || assignment->useLet) {
                    context->codeNode = nullptr;
                    context->scanEnd = true;

                    return context->prevFoundPos; // revert to name
                } else {
                    return -1;
                }

            }
        } else {
            int result;
            if (-1 < (result = Tokenizers::jsonValueTokenizer(Cast::upcast(assignment), ch,
                                                              start, context))) {
                assignment->valueNode = context->codeNode;
                context->scanEnd = true;

                return result;
            }
        }

        return -1;
    }



    int Tokenizers::assignStatementWithoutLetTokenizer(TokenizerParams_parent_ch_start_context)
    {
        AssignStatementNodeStruct *assignment;

        if (context->unusedAssignment == nullptr) {
            assignment = Alloc::newAssignStatement(context, parent);
        } else {
            assignment = context->unusedAssignment;
            context->unusedAssignment = nullptr;
        }

        int resultPos;
        if (-1 < (resultPos = Scanner::scanMulti(assignment, inner_assignStatementTokenizerMulti,
                                                 start, context))) {

            assignment->useMut = false;
            assignment->useLet = false;

            context->codeNode = Cast::upcast(&assignment->nameNode);
            context->virtualCodeNode = Cast::upcast(assignment);

            return resultPos;

        }

        context->unusedAssignment = assignment;

        return -1;
    }

    // let a = 3
    // mut m = 5
    // m = 8
    int Tokenizers::assignStatementTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char let_chars[] = "let";
        static constexpr unsigned int size_of_let = sizeof(let_chars) - 1;

        static constexpr const char mut_chars[] = "mut";
        static constexpr unsigned int size_of_mut = sizeof(mut_chars) - 1;

        bool hasLet = false;
        bool hasMut = false;
        int currentPos = start;

        // let
        if ('l' == ch) {
            auto idx = ParseUtil::matchAt(context->chars, context->length, start, let_chars);
            if (idx > -1) {
                currentPos = idx + size_of_let;
                hasLet = true;
            }
        }

        if (!hasLet) { // mut
            if ('m' == ch) {
                auto idx = ParseUtil::matchAt(context->chars, context->length, start, mut_chars);
                if (idx > -1) {
                    currentPos = idx + size_of_mut;
                    hasMut = true;
                }
            }
        }


        if (hasMut || hasLet) {
            AssignStatementNodeStruct *assignStatement;
            if (context->unusedAssignment == nullptr) {
                assignStatement = Alloc::newAssignStatement(context, parent);
            } else {
                assignStatement = context->unusedAssignment;
                context->unusedAssignment = nullptr;
            }

            assignStatement->useMut = hasMut;
            assignStatement->useLet = hasLet;

            int resultPos;
            if (-1 < (resultPos = Scanner::scanMulti(assignStatement, inner_assignStatementTokenizerMulti,
                                                currentPos, context))) {

                context->codeNode = Cast::upcast(&assignStatement->letOrMut);
                context->virtualCodeNode = Cast::upcast(assignStatement);


                assignStatement->letOrMut.text = context->memBuffer.newMem<char>(3 + 1);
                assignStatement->letOrMut.textLength = 3;

                memcpy((char*)assignStatement->letOrMut.text, hasMut?(char*)"mut":(char*)"let", 3);
                assignStatement->letOrMut.text[3] = '\0';

                return resultPos;
            } else {
                context->unusedAssignment = assignStatement;
                return -1;
            }
        }

        return -1;
    }
}


