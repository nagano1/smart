#include <string>
#include <mutex>
#include <thread>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>

#include <cstdint>
#include <ctime>
//#include <android/log.h>

//#include "../../../../../tests/cplusplus_test.h"

#include "test_common.h"

#include "testlib_impl.hpp"

long Test2::g = 59;



namespace smartlang {

    class TestFactoryNode {
    public:
        TestFactoryNode(TestFactoryBase *factory, std::string argtestcasename,
            std::string argtestname)
            : testcasename{ argtestcasename }, testname{ argtestname } {
            this->factory = factory;
        }

        std::string testcasename;
        std::string testname;

        TestFactoryBase *factory = nullptr;
        TestFactoryNode *next = nullptr;
    };

    TestFactoryNode *lastTestFactory = nullptr;
    TestFactoryNode *firstTestFactory = nullptr;

    static std::unordered_map<std::string, TestFactoryNode *> testFactories;// = new std::vector<TestFactoryBase *>;


    int *CPPTestUtil::MakeAndRegisterTestInfo(
        const char *testcasename, const char *testname, const char *file, int line,
        TestFactoryBase *factory
    ) {

        printf("test:%s\n", testname);
        std::string st = file;

        //if ("test_casewow" == std::string(testcasename)) {
        //__android_log_print(ANDROID_LOG_DEBUG, "aaa", "Message : %s, %d", st.c_str(), line);

        auto *node = new TestFactoryNode(factory, testcasename, testname);
        if (firstTestFactory == nullptr) {
            firstTestFactory = node;
        }
        if (lastTestFactory != nullptr) {
            lastTestFactory->next = node;
        }
        lastTestFactory = node;
        //}
        return new int(3);
    };


    std::string make_key(const char *testcasename, const char *testname) {
        std::string key = std::string(testcasename) + '_' + std::string(testname);
        //return std::move(key);
        return key;
    }

    void initTests() {
        if (testFactories.size() == 0) {

            auto *current = firstTestFactory;
            while (current != nullptr) {
                std::string key = make_key(current->testcasename.c_str(),
                    current->testname.c_str());
                testFactories.insert(
                    std::make_pair(key, current));

                current = current->next;
            }
        }
    }


    static unsigned int current_test_index = 0;
    static smartlang::Test *currentTest = nullptr;
    static std::string last_text = "";
    static std::mutex stream_mtx;

    static bool failed = false;

    int CPPTestUtil::runTest(const char *testcasename, const char *testname) {

        initTests();
        failed = false;

        auto key = make_key(testcasename, testname);
        if (testFactories.count(key) > 0) {
            auto *current = testFactories.at(key);

            current_test_index++;

            auto tha = std::thread([](int test_index, TestFactoryNode *current) {
                auto *test = current->factory->CreateTest();
                currentTest = test;
                test->runTestBody();

                if (test_index == current_test_index) {
                    failed = test->failed;
                    std::lock_guard<std::mutex> g{ Test::log_mtx };
                    std::stringstream out;
                    std::copy(std::istreambuf_iterator<char>(Test::logstream),
                        std::istreambuf_iterator<char>(),
                        std::ostream_iterator<char>(out));

                    last_text = out.str();

                    currentTest = nullptr;
                    current_test_index++;
                }

                delete test;


            }, current_test_index, current);

            tha.detach();

            return current_test_index;
        }

        return 0;
    }

    std::string CPPTestUtil::runTestNext(int testIndex) {

        initTests();

        {
            std::lock_guard<std::mutex> g{ Test::log_mtx };
            if (current_test_index == testIndex) {

                std::stringstream out;
                std::copy(std::istreambuf_iterator<char>(Test::logstream),
                    std::istreambuf_iterator<char>(),
                    std::ostream_iterator<char>(out));
                auto &&str = out.str();
                if (str.length() > 0) {
                    return std::move(str);
                }
                return "";
            }

            if (last_text.length() > 0) {
                auto temp = last_text;
                last_text = "";
                if (temp.length() > 0) {
                    return temp;
                }
                return "";
            }

        }

        if (failed) {
            return "[__end_of_test_0];";
        }
        else {
            return "[__end_of_test_1];";
        }
    }

    void *CPPTestUtil::doAllTests() {
        initTests();

        for (auto &&testFactory : testFactories) {
            auto *test = testFactory.second->factory->CreateTest();
            test->TestBody();
        }

        return nullptr;
    }

    TestInfo::~TestInfo() {

    }

    Test *Test::this_current_test = nullptr;
    std::stringstream smartlang::Test::logstream;
    std::mutex smartlang::Test::log_mtx;


    Test::~Test() {

    }
}

TEST(smartest, winotest) {
    EXPECT_EQ(54, 54.0f);
    EXPECT_EQ(2.3f, 5.4f-2.1f);
}

ENDTEST

