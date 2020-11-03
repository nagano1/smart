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



//
//struct FuncNode::FuncBodyNode : CodeNodeBase {
//    bool isBodyEnd = false;
//    FuncBodyNode *endBodyNode = nullptr;
//    std::vector<CodeNodeBase *> expressionNodes;
//
//    FuncBodyNode(CodeNodeBase *parentNode) : CodeNodeBase{ parentNode } {
//
//    }
//
//    virtual ~FuncBodyNode() {
//        if (endBodyNode) {
//            delete endBodyNode;
//        }
//    };
//
//    virtual bool overrideTemplateStub(const std::string &text) override {
//        return true;
//    };
//
//
//    virtual CodeLine *appendToLine(CodeLine *currentCodeLine) override {
//        currentCodeLine = this->addLineBreakNode(currentCodeLine);
//        currentCodeLine->nodes.push_back(this);
//        if (this->endBodyNode) {
//            currentCodeLine = this->endBodyNode->addLineBreakNode(currentCodeLine);
//            currentCodeLine->nodes.push_back(this->endBodyNode);
//        }
//        return currentCodeLine;
//    }
//
//    virtual std::string textWithoutChildren() override {
//        return isBodyEnd ? "}" : "{";
//    };
//};
//
//
//
//struct FuncBodyTokenizer {
//
//    FuncNode::FuncBodyNode *bodyNode = nullptr;
//
//    inline int tryTokenize(FuncNode *parent, utf8byte ch, int start, ParseContext *context) {
//
//        unsigned int found_count = 0;
//        if (bodyNode == nullptr) {
//            if (ch == '{') {
//                int returnPosition = start + 1;
//
//                bodyNode = new FuncNode::FuncBodyNode(parent);
//
//                /*
//                {
//                    NameNode::NameTokenizer nameTokenizer;
//                    //  class AClass
//                    int result = scanChars(bodyNode, &nameTokenizer, returnPosition, context);
//                    if (result > -1) {
//                        bodyNode->expressionNodes.push_back(nameTokenizer.codeNode);
//                        returnPosition = result;
//                    }
//                }
//                */
//
//                context->codeNode = bodyNode;
//                return returnPosition;
//            }
//        }
//        else if (ch == '}') {
//            context->scanEnd = true;
//            bodyNode->endBodyNode = new FuncNode::FuncBodyNode(bodyNode);
//
//            bodyNode->endBodyNode->isBodyEnd = true;
//            context->codeNode = bodyNode->endBodyNode;
//            return start + 1;
//        }
//
//        return -1;
//    }
//};
//
//
///**
// * FuncNode
// */
//bool FuncNode::overrideTemplateStub(const std::string &text) {
//    return text == "wowowowow";
//}
//
//
//static constexpr const char *fn_chars = "fn";
//static constexpr int fn_chars_length = 2; // std::char_traits<char>::length(fn_chars);
//
//int FuncNode::FuncTokenizer::tryTokenize(CodeNodeBase *parentNode,
//                                         utf8byte first_ch,
//                                         int i, ParseContext *context) {
//    if ('f' == first_ch) {
//        auto idx = Tokenizer::matchFirstWithTrim(context->chars, fn_chars, i);
//        if (idx > -1) {
//            if (Tokenizer::isSpace(context->chars[idx + fn_chars_length])) {
//                int returnPosition = idx + fn_chars_length;
//
//                // "class " came, generate ClassNode
//                funcNode = new FuncNode(parentNode);
//                context->codeNode = funcNode;
//
//                {
//                    // Tokenize class Name, class AClass
//                    NameNode<FuncNode>::NameTokenizer nameTokenizer;
//                    int result = scanChars(funcNode, nameTokenizer, returnPosition, context, /*not root*/false);
//                    if (result == -1) {
//                        context->codeNode = funcNode;
//                        return returnPosition;
//                    }
//                    funcNode->funcName = nameTokenizer.nameNode;
//                    returnPosition = result;
//                }
//
//
//                {
//                    //  Tokenize body  {}
//                    FuncBodyTokenizer bodyTokenizer;
//                    int result = scanChars(funcNode, bodyTokenizer, returnPosition, context, false);
//                    if (result == -1) {
//                        context->codeNode = funcNode;
//                        return returnPosition;
//                    }
//
//                    funcNode->bodyNode = bodyTokenizer.bodyNode;
//                    returnPosition = result;
//                }
//
//                context->codeNode = funcNode;
//                return returnPosition;
//            }
//        }
//    }
//
//    return -1;
//};
//
//CodeLine *FuncNode::appendToLine(CodeLine *currentCodeLine) {
//    currentCodeLine = addLineBreakNode(currentCodeLine);
//    currentCodeLine->nodes.push_back(this);
//    if (funcName) {
//        currentCodeLine = funcName->appendToLine(currentCodeLine);
//    }
//    if (bodyNode) {
//        currentCodeLine = bodyNode->appendToLine(currentCodeLine);
//    }
//    return currentCodeLine;
//
//}
