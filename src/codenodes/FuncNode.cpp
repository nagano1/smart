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




/**
 * FuncNode
 */

static constexpr const char *fn_chars = "fn";
static constexpr int fn_chars_length = 2; // std::char_traits<char>::length(fn_chars);

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
                    int result = scanChars(funcNode, nameTokenizer, returnPosition, context, /*not root*/false);
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
