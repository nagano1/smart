//
//  cpptest.cpp
//

#include "cpptest.hpp"
#include "../../tests/testlib_impl.hpp"


TEST(testrer2, maoiwjoafw) {
    
    
    
}

//ENDTEST

std::vector<TestFactoryBase *> testFactories;

int* CPPTestUtil::MakeAndRegisterTestInfo(const char* testcasename, const char* testname,TestFactoryBase *factory){
    testFactories.push_back(factory);
    
    return new int(3);
};


TestInfo::~TestInfo() {

    
}


Test::~Test() {

}

class AB {

};

TEST(my_test_case_a, print_test)
{
    float ff = 2.3;
    EXPECT_EQ(100, ff);

}

TEST(my_test_case_a, print_test2)
{
    
}

constexpr int BUFFER_SIZE = 16;


const char* Cpp::runTestNext(int testId) {
    auto *str_p = new std::string(smartlang::CPPTestUtil::runTestNext(testId));

    return str_p->c_str();
}

int Cpp::runTest(const char *testcasename, const char* testname) {

    int testId = smartlang::CPPTestUtil::runTest(testcasename, testname);
    return testId;

    //while (true) {

}

void Cpp::run(const char* utf8chars) {
    int i = -1;
    std::vector<char> chars;
    while (true) {
        i++;
        if (utf8chars[i] == '\0'){
            break;
        }
        
        chars.push_back(utf8chars[i]);
        printf("%c\n", utf8chars[i]);
    }
    
    chars.push_back('\0');
    auto *te = &chars[0];
    printf("%s", te);


}

