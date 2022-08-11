#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "script_runtime/script_runtime.hpp"

using namespace smart;


int main()
{
    printf("Smart Lang v0.0.1\n");

    constexpr char text[] = R"(
fn main() {
    let a = 0
    int b = 0
    print("test日本語")
}
)";

    const char* chars = text;// .c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
    DocumentUtils::parseText(document, chars, sizeof(text)-1);
    DocumentUtils::generateHashTables(document);

    char* treeText = DocumentUtils::getTextFromTree(document);
    //printf("%s\n", treeText);

    startScript(document);


    Alloc::deleteDocument(document);

    /*
    auto* item = document->hashMap->get2("method");

//    item->vtable, VTables::StringLiteralVTable

    auto* strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
    strNode->strLength;
    strNode->textLength;


    */

    printf("\n\n...end\n");
}
