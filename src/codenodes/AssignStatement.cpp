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
        // virtual node
        return 0;
    }

    static const utf8byte *selfText(AssignStatementNodeStruct *self) {
        // virtual node
        return ""; // self->useMut ? "mut" : "let";
    }

    static CodeLine *appendToLine(AssignStatementNodeStruct *self, CodeLine *currentCodeLine) {

        if (!self->onlyAssign) {
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


        assignStatement->hasMutMark = false;
        assignStatement->useLet = false;
        assignStatement->onlyAssign = false;

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
                //if (assignment->assignment->useLet) {
                    context->codeNode = nullptr;
                    context->scanEnd = true;

                    return context->prevFoundPos; // revert to name
                //} else {
//                    return -1;
//                }

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


    // b = 32
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

            assignment->onlyAssign = true;
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

        bool hasLet = false;
        int currentPos = start;

        bool hasMutMark = false;

        AssignStatementNodeStruct *assignStatement;
        if (context->unusedAssignment == nullptr) {
            assignStatement = Alloc::newAssignStatement(context, parent);
        } else {
            assignStatement = context->unusedAssignment;
            context->unusedAssignment = nullptr;
        }



        // $ is mutable mark
        if ('$' == ch) {
            hasMutMark = true;
            currentPos += 1;
        }

        int found_count = 0;

        // let
        if ('l' == ch) {
            auto idx = ParseUtil::matchAt(context->chars, context->length, currentPos, let_chars);
            if (idx > -1) {
                currentPos = idx + size_of_let;
                hasLet = true;
                found_count = 3;
            }
        }

        if (!hasLet) {
            for (uint_fast32_t i = currentPos; i < context->length; i++) {
                if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                    found_count++;
                } else {
                    break;
                }
            }

            if (found_count > 0) {
                if (currentPos + found_count < context->length
                    && ParseUtil::isSpaceOrLineBreak(context->chars[currentPos + found_count])
                        ){
            } else {
                    found_count = 0;
                }
            }
            currentPos += found_count;
        }

        if (found_count > 0) {
            Init::assignText_SimpleTextNode(&assignStatement->letOrMut, context, start,
                                            found_count + (hasMutMark ? 1 : 0));

            assignStatement->onlyAssign = false;
            assignStatement->useLet = hasLet;

            int resultPos;
            if (-1 < (resultPos = Scanner::scanMulti(assignStatement,
                                                     inner_assignStatementTokenizerMulti,
                                                     currentPos, context))) {


                context->codeNode = Cast::upcast(&assignStatement->letOrMut);
                context->virtualCodeNode = Cast::upcast(assignStatement);

                return resultPos;
            }
        }

        context->unusedAssignment = assignStatement;

        return -1;
    }
}



