#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "parse_util.hpp"

using namespace smart;


int main()
{
    printf("Smart Lang v0.0.1\n");

    std::string text = u8R"(
class A {
    class B {
        class TestCl😂日本語10234ass
        {
            fn func()
            {
                1234123
                "jfoiwaejio"
                null
                2134123

                1234123
                "jfoiwaejio"
                2134123

                let c = 0
                let d = 893214
                $let intA = 314

                let c = 0
                let d = 893214
            }
        }
    }
}
class A {}
class A {}
class A {}
class BDD{}



class AABC  {  }

)";

    const char* chars = text.c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
    DocumentUtils::parseText(document, chars, text.size());

    DocumentUtils::generateHashTables(document);

    /*
    auto* item = document->hashMap->get2("method");

//    item->vtable, VTables::StringLiteralVTable

    auto* strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
    strNode->strLength;
    strNode->textLength;

    //char* treeText = DocumentUtils::getTextFromTree(document);

    Alloc::deleteDocument(document);
    */

    printf("[fwe2]");
}
