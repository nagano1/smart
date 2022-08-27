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

    static int selfTextLength(AssignStatementNodeStruct *) {
        // virtual node
        return 0;
    }

    static const utf8byte *selfText(AssignStatementNodeStruct *self) {
        // virtual node
        return ""; // self->useMut ? "mut" : "let";
    }


    static CodeLine *appendToLine(AssignStatementNodeStruct *self, CodeLine *currentCodeLine) {

        if (!self->onlyAssign) {
            currentCodeLine = VTableCall::appendToLine(&self->typeOrLet, currentCodeLine);
        }

        if (self->pointerAsterisk.found > -1) {
            currentCodeLine = VTableCall::appendToLine(&self->pointerAsterisk, currentCodeLine);
        }

        currentCodeLine = VTableCall::appendToLine(&self->nameNode, currentCodeLine);

        if (self->equalSymbol.found > -1) {
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
        Init::initAssignStatement(context, parentNode,  assignStatement);
        return assignStatement;
    }

    void Init::initAssignStatement(ParseContext *context, NodeBase *parentNode, AssignStatementNodeStruct *assignStatement) {
        INIT_NODE(assignStatement, context, parentNode, &_assignVTable);

        assignStatement->onlyAssign = false;
        assignStatement->hasType = false;
        assignStatement->valueNode = nullptr;


        Init::initSymbolNode(&assignStatement->pointerAsterisk, context, assignStatement, '*');

        Init::initNameNode(&assignStatement->nameNode, context, assignStatement);
        Init::initSymbolNode(&assignStatement->equalSymbol, context, assignStatement, '=');
        Init::initTypeNode(&assignStatement->typeOrLet, context, assignStatement);
    }


    static int inner_assignStatementTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *assignment = Cast::downcast<AssignStatementNodeStruct *>(parent);

        // console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());

        if (assignment->nameNode.found == -1) {
             if (assignment->hasType && context->afterLineBreak) {
                 return -1;
             }

             if (assignment->pointerAsterisk.found == -1) {
                if (ch == '*') {
                    assignment->pointerAsterisk.found = start;
                    context->setCodeNode(&assignment->pointerAsterisk);
                    return start + 1;
                }
            }

            int result;
            if (-1 < (result = Tokenizers::nameTokenizer(Cast::upcast(&assignment->nameNode)
                                                        , ch, start, context))
            ) {
                assignment->nameNode.found = result;
                context->setCodeNode(&assignment->nameNode);
                return result;
            }
            else {
                //context->scanEnd = true;
                //context->setError(ErrorCode::syntax_error, start);

            }
        }
        else if (assignment->equalSymbol.found == -1) {
            if (ch == '=') {
                assignment->equalSymbol.found = start;
                context->setCodeNode(&assignment->equalSymbol);
                return start+1;
            }
            else {
                if (assignment->hasType) {
                    context->setCodeNode(nullptr);
                    context->scanEnd = true;
                    return context->prevFoundPos;// assignment->nameNode.found;//context->prevFoundPos; // revert to name
                }
                //else {
                    //context->scanEnd = true;
                    //context->setError(ErrorCode::syntax_error, start);
//                    return -1;
                //}
            }
        }
        else {
            int result;
            if (-1 < (result = Tokenizers::expressionTokenizer(Cast::upcast(assignment), ch,
                                                               start, context))) {
                assignment->valueNode = context->virtualCodeNode;
                context->scanEnd = true;
                return result;
            }
            else {
                //context->scanEnd = true;
                //context->setError(ErrorCode::syntax_error, start);
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
        }
        else {
            assignment = context->unusedAssignment;
            Init::initAssignStatement(context, parent, assignment);
            context->unusedAssignment = nullptr;
        }

        int resultPos;
        if (-1 < (resultPos = Scanner::scanMulti(assignment, inner_assignStatementTokenizerMulti,
                                                 start, context))) {
            assignment->onlyAssign = true;
            assignment->typeOrLet.useLet = false;

            context->leftNode = Cast::upcast(&assignment->nameNode);
            context->virtualCodeNode = Cast::upcast(assignment);

            return resultPos;
        }

        context->unusedAssignment = assignment;

        return -1;
    }

    // let a = 3
    // $int m = 5
    // int a
    int Tokenizers::assignStatementTokenizer(TokenizerParams_parent_ch_start_context)
    {
        AssignStatementNodeStruct *assignStatement;
        if (context->unusedAssignment == nullptr) {
            assignStatement = Alloc::newAssignStatement(context, parent);
        }
        else {
            assignStatement = context->unusedAssignment;
            Init::initAssignStatement(context, parent, assignStatement);
            context->unusedAssignment = nullptr;
        }

        int resul = Tokenizers::typeTokenizer(Cast::upcast(&assignStatement->typeOrLet), ch, start, context);
        if (resul > -1) {
            assignStatement->onlyAssign = false;
            assignStatement->hasType = true;

            context->afterLineBreak = false;
            int resultPos;
            if (-1 < (resultPos = Scanner::scanMulti(assignStatement,
                                                     inner_assignStatementTokenizerMulti,
                                                     resul, context))
                    ) {
                context->leftNode = Cast::upcast(&assignStatement->typeOrLet);
                context->virtualCodeNode = Cast::upcast(assignStatement);

                return resultPos;
            }
        }

        context->unusedAssignment = assignStatement;

        return -1;
    }
}



