#include <stdlib.h>
#include <string.h>
#include "code_nodes.hpp"
#include "parse_util.hpp"

using namespace smart;

using namespace smart;

using i32 = uint32_t;

extern "C" i32 print_num(i32 n);
extern "C" i32 do_nothing();
extern "C" void print(const char *n);
extern "C" i32 new_address_map();
extern "C" void set_address_map(i32 map_id, i32 key, i32 val);
extern "C" i32 get_address_map(i32 map_id, i32 val);
extern "C" void consoleTime();
extern "C" void consoleTimeEnd();
extern "C" i32 timeSeed();
extern "C" void *memory;

class A
{
public:
    int g = 555555;
    int func()
    {
        return 9992;
    }
};

struct SpaceClass
{
    SpaceClass()
    {
    }

    bool overrideTemplateStub(const std::string &text)
    {
        return true;
    };
};

/**
 * To provide lazy-load of wasm-module to improve UX and achive addon/plugin system.
 * Provides Shared heap memory through multiple wasm-modules
 *
 * 1. Allocating is always executed on one module called   loaded first on webpage.
 *
 *
 */
struct MemInfo
{
    int a = 3214;
    char *v = nullptr;
};

extern "C"
{
    char *get_memory_for_string(int length)
    {
        char *a = (char *)malloc(length);
        return a;
    }
    void free_memory_for_string(char *p)
    {
        free(p);
    }
    int string_to_int(char *p)
    {
        int length = 0;
        for (int i = 0; i < 100000; i++)
        {
            if (p[i] == '\0')
            {
                break;
            }
            length++;
        }
        int result = 0;
        int multiplier = 1;
        for (int i = length - 1;; i--)
        {
            result += multiplier * (p[i] - '0');
            if (i < 1)
            {
                break;
            }
            multiplier = multiplier * 10;
        }

        return result;
    }

    int calling(int p)
    {

        char *entries[256];
        for (int i = 0; i < 256; i++) {
            entries[i] = nullptr;
        }
        for (int i = 0; i < 256; i++) {
            if (entries[i] != nullptr) {
                throw 3;
            }
        }

        print("{\"isTestResult\":true, \"ok\": true}");
        print("then, what got there?");
        void *a = malloc(3);

        print_num(998234);
        print_num((int)a);

        //return 3;

        //return 3;
        const char *text2 = R"(                                                         
                                                                                    
{                                                                                   
                                                                                    
    "null = ": null,                                                                
                                                                                    
    "jiofw":12,                                                                     
                                                                                    
    "nest": {}                                                                      
                                                                                    
}                                                                                   
                                                                                    
  )";

        auto *doc = Alloc::newDocument(DocumentType::JsonDocument, nullptr);

        DocumentUtils::parseText(doc, text2, strlen(text2));

        char *treeText = DocumentUtils::getTextFromTree(doc);

        print(treeText);

        const char *text = R"(
class TestCl😂日本語10234ass {

}


fn      myfunc1 {


}
fn myfunc  {

}

class aw{
}
class kaw{
}
  )";

        //CodeLine *codeLine = new CodeLine();

        //const char *chars = text.c_str();

        //RootCodeManager manager{};
        //std::string text = "   class           A   {    }   ";

        //codeLine = new CodeLine();

        //print_num((uintptr_t)codeLine);
        //std::string a_str;
        //print(a_str.c_str());
        //
        //std::vector<int> int_vec;
        //int_vec.push_back(2);
        // print_num((uintptr_t)&int_vec);

        print("{\"isTestResult\":true, \"ok\": true}");

        if (34 == 34)
        {
            //print("{\"isTestResult\":true, \"ok\": false}");
        }
        //printf("RAND_MAX: %d\n", RAND_MAX);

        print("time seed = ");
        print_num(timeSeed());
        srand(timeSeed());
        //srand((unsigned int)time(NULL));

        consoleTime();
        unsigned int bg = 0;

        for (int i = 0; i < 1 * 1000000; i++)
        {
            if (i > 5)
            {
                bg += do_nothing();
            }
            //bg+=(rand());
        }
        consoleTimeEnd();
        print_num(bg);
        consoleTime();
        for (int i = 0; i < 10 * 1000; i++)
        {
            auto *doc = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
            //std::unique_ptr<SpaceClass> afw{new SpaceClass()};
            //print_num((uintptr_t)afw.get());
            DocumentUtils::parseText(doc, text, strlen(text));
            //print_num(233);
            //print_num(doc->nodeCount);
            //print_num(233);

            Alloc::deleteDocument(doc);
            //free(doc);
            //DocumentVTable->init((NodeBase*)&doc);
            //parseText(&document, chars, text.size());
            //EXPECT_EQ(doc.rootNodes.size(), 4);

            //doc.appendLineNode(codeLine);
        }

        consoleTimeEnd();

        char str1[128] = {1, 1, 1, 1, 5, 1, 1};
        char str2[] = "ab\0cde";
        int i;

        /*日本語*/
        print("joifewo \n");
        for (i = 0; i < 7; i++)
            print_num(str1[i]);
        print("\n");

        memcpy(str1, str2, 5);

        //clock_t start = clock();

        uint64_t loopCount = 1000 * 10 * 1000LLU;
        uint64_t current = 255;

        uintptr_t val;

        i32 map_id = new_address_map();
        {
            for (int i = 0; i < 1000 * 1000; i++)
            {
                //print_num(580032*i);
                if (i > 55)
                {

                    /*
                A * a = new A();//aobj{};
                        val = i;
                        set_address_map(map_id, i, (uintptr_t)a);
                delete a;
                */
                    //val = get_address_map(map_id, i);
                    //vec.push_back(3);
                }
            }
        }

        /*
    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    print_num((int)one_op_nanosec);
    */

        /*
        char * ch = new char[]{"haoiwjee2"};
     print(ch);
     print("______joifwe");

     val = get_address_map(map_id, 1000);
     print_num(val);

     char *result = static_cast<char*>(malloc(1024));
     A * a = new A();//aobj{};
             //strncpy(result, "jifow", strlen("jifow"));

                    print_num((uintptr_t)a);
                    delete a;
                    */

        /*
     print_num((int)result);
     i32 map_id = new_address_map();
     set_address_map(map_id, 155, (uintptr_t)a);
     uintptr_t val = get_address_map(map_id, 155);
     A* getP = (A*)val;
     getP->g;
     print_num(getP->g);
                    delete a;
     std::unordered_map<int, int> um;
     um.insert(std::pair<int,int>{3,5});

     */

        print(" a nothing has happend.? !\n");

        return 955;
        //return *aaa;//a->func();//*ii;;
    }
}

extern "C" int _start()
{
    for (int i = 0; i < 100; i++)
    {
        print_num(58);
    }
    //std::unordered_map<int, int> um;
    return 554;
}
