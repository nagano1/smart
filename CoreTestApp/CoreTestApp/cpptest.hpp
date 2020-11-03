//
//  cpptest.hpp
//

#ifndef cpptest_hpp
#define cpptest_hpp

#include <stdio.h>

#include <array>
#include <vector>
#include <atomic>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <algorithm>

#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <cstdint>

class TestInfo
{
public:
    // Destructs a TestInfo object.  This function is not virtual, so
    // don't inherit from TestInfo.
    ~TestInfo();

    // Returns the test case name.
    const char *test_case_name() const { return test_case_name_.c_str(); }

    // Returns the test name.
    const char *name() const { return name_.c_str(); }

    // Returns the name of the parameter type, or NULL if this is not a typed
    // or a type-parameterized test.
    const char *type_param() const
    {
        if (type_param_.get() != NULL)
            return type_param_->c_str();
        return NULL;
    }

    // Returns the text representation of the value parameter, or NULL if this
    // is not a value-parameterized test.
    const char *value_param() const
    {
        if (value_param_.get() != NULL)
            return value_param_->c_str();
        return NULL;
    }

private:
    const std::unique_ptr<const ::std::string> type_param_;
    // Text representation of the value parameter, or NULL if this is not a
    // value-parameterized test.
    const std::unique_ptr<const ::std::string> value_param_;

    const std::string test_case_name_; // Test case name
    const std::string name_;           // Test name
    bool should_run_;                  // True iff this test should run
    bool is_disabled_;                 // True iff this test is disabled
    bool matches_filter_;              // True if this test matches the
};

class Test
{
public:
    friend class TestInfo;

    // Defines types for pointers to functions that set up and tear down
    // a test case.
    //typedef internal::SetUpTestCaseFunc SetUpTestCaseFunc;
    //typedef internal::TearDownTestCaseFunc TearDownTestCaseFunc;

    // The d'tor is virtual as we intend to inherit from Test.
    virtual ~Test();
};

class TestFactoryBase
{

public:
    virtual ~TestFactoryBase() {}

    // Creates a test instance to run. The instance is both created and destroyed
    // within TestInfoImpl::Run()
    virtual Test *CreateTest() = 0;

protected:
    TestFactoryBase() {}
};

// This class provides implementation of TeastFactoryBase interface.
// It is used in TEST and TEST_F macros.
template <class TestClass>
class TestFactoryImpl : public TestFactoryBase
{
public:
    virtual Test *CreateTest() { return new TestClass; }
};

class CPPTestUtil
{
public:
    static int *MakeAndRegisterTestInfo(const char *testcasename, const char *testname, TestFactoryBase* factory);
};


// Expands to the name of the class that implements the given test.
#define GTEST_TEST_CLASS_NAME_(test_case_name, test_name) \
    test_case_name##_##test_name##_Test

// Helper macro for defining tests.
#define GTEST_TEST_(test_case_name, test_name, parent_class)                      \
    class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class \
    {                                                                             \
    public:                                                                       \
        GTEST_TEST_CLASS_NAME_(test_case_name, test_name)                         \
        () {}                                                                     \
        ~GTEST_TEST_CLASS_NAME_(test_case_name, test_name)                         \
        (){}                                                                       \
    private:                                                                      \
        static int *test_info_;                                                   \
        void TestBody();                                                          \
    };                                                                            \
                                                                                \
    int *GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::test_info_ =          \
        CPPTestUtil::MakeAndRegisterTestInfo(#test_case_name, #test_name,         \
        new TestFactoryImpl<GTEST_TEST_CLASS_NAME_(test_case_name, test_name)> ); \
                                                                                  \
    void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::TestBody()

#define GTEST_TEST(test_case_name, test_name) \
    GTEST_TEST_(test_case_name, test_name, Test)

#define TEST(test_case_name, test_name) GTEST_TEST(test_case_name, test_name)


template<typename T1, typename T2>
static void ecompare_static(T1 t1, T2 t2) {
    if (t1 == t2) {

    } else {

    }
}

template<>
static void ecompare_static(float t1, float t2) {
    if (t1 <= t2) {

    } else {

    }
}



#define EXPECT_EQ(val1, val2) \
ecompare_static(val1, val2)

/**
 *  〇 -> 〇 -> 〇
 *   |     |
 *  〇    〇
 */
class GreenThread;

class Cpp
{
public:
    void run(const char *utf8chars);
    int runTest(const char *testcasename, const char* testname);
    const char* runTestNext(int testId);
};

class RunNode
{
public:
    std::atomic<RunNode *> next{nullptr};
    std::atomic<bool> scheduled{false};
    int idx = 0;
};

class ThreadQueue
{
public:
    std::atomic<RunNode *> lastRunNode{nullptr};
    std::atomic<RunNode *> doingRunNode{nullptr};
};

constexpr int thread_queue_size = 2;
constexpr int threads_size = 2;

class ThreadPool
{
public:
    std::vector<GreenThread *> threads;
    std::array<ThreadQueue *, thread_queue_size> thread_queue;
    int init();
    int start();
};

class GreenThread
{
public:
    std::condition_variable main_sleeper_cond;
    std::atomic<bool> main_sleeper_ready{false};
    std::mutex main_sleeper_mutex;

    ThreadPool *threadPool;

    int prev_idx = 0;

    int idx;
    int set_count;

    int init(int idx3);

    volatile bool stop{false};
    std::thread *tha;

    int currentRunNodeIndex = 1000000; // should be larger than buffer size

    RunNode *buffer;
    RunNode *getNextRunNode();

    void start();
    void join();
    void doStop();
};

#endif /* cpptest_hpp */
