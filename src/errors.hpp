#pragma once
//
#include <stdlib.h>
#include <array>
//
#include <cstdlib>
//#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
//
#include <cstdint> // uint64_t, int_fast32_t
#include <ctime>
//
#include <string.h> // memcpy
//
//
////using utf8byte = char;
///*
// *
// * afunc
// *   <int<int>>
// *   () {
// *
// * }
// *
// *
// */
namespace smart {

    enum class ErrorCode {
        first_keeper,

        //----------------------------------------------------------------------------------
        //
        //                                  Syntax Errors
        //
        //----------------------------------------------------------------------------------

        no_syntax_error,

        // common
        syntax_error,
        syntax_error2,
        should_break_line,
        indent_error,

        // value
        expect_end_parenthesis,


        // string
        missing_closing_quote,
        missing_closing_quote2,
        missing_object_delemeter,

        // class
        invalid_class_name,
        no_brace_for_class,
        no_brace_of_end_for_class,

        // fn
        invalid_fn_name,
        expect_bracket_for_fn_body,
        expect_parenthesis_for_fn_params,
        expect_end_parenthesis_for_fn_params,

        // return
        no_value_for_return,

        //----------------------------------------------------------------------------------
        //
        //                                  Logical Errors
        //
        //----------------------------------------------------------------------------------
        no_logical_error,
        no_variable_defined,


        last_keeper
    };

    template<
        typename tEnum,
        typename std::enable_if<std::is_enum<tEnum>::value, std::nullptr_t>::type = nullptr
    >
        std::ostream& operator<<(std::ostream& iOStream, tEnum iEnum)
    {
        typedef typename std::underlying_type<tEnum>::type  Type;
        iOStream << static_cast<Type>(iEnum);
        return iOStream;
    }


    static constexpr int errorListSize = 1 + static_cast<int>(ErrorCode::last_keeper);


    struct ErrorInfo {
        ErrorCode errorIndex;
        int errorCode;
        const char* msg;

        static ErrorInfo ErrorInfoList[errorListSize];
        static bool errorInfoInitialized;
    };


    static int acompare(void const * alhs, void const * arhs) {
        ErrorInfo* lhs = (ErrorInfo*)alhs;
        ErrorInfo* rhs = (ErrorInfo*)arhs;

        if (lhs->errorCode == rhs->errorCode) {
#if defined(_MSVC_LANG) //_MSC_VER  _MSVC_LANG _MSC_BUILD
            printf("duplicate error id(%d)\n ", lhs->errorCode);
#endif
            //exit(9990);
            return 0;
        }
        else if (lhs->errorCode > rhs->errorCode) {
            return 1;
        }
        else {
            return -1;
        }
        //return lhs->errorCode - rhs->errorCode;
        return 0;
    }


    // C++-14
    /*
    static constexpr bool is_sorted(const ErrorInfo tempList[])
    {
        for (std::size_t i = 0; i < errorListSize - 1; ++i) {
            if (tempList[i].errorCode >= tempList[i + 1].errorCode) {
                return false;
            }
        }
        return true;
    }
    */




    static int initErrorInfoList()
    {
        ErrorInfo::errorInfoInitialized = true;

        static ErrorInfo tempList[] = {
            ErrorInfo{ ErrorCode::first_keeper, 9912, "start"},

            //----------------------------------------------------------------------------------
            //
            //                                     Syntax Errors
            //
            //----------------------------------------------------------------------------------
            ErrorInfo{ ErrorCode::no_syntax_error, 10000, "no_error"},

            // common
            ErrorInfo{ ErrorCode::syntax_error, 418030, "syntax error" },
            ErrorInfo{ ErrorCode::syntax_error2, 418031, "syntax error2" },
            ErrorInfo{ ErrorCode::should_break_line, 418032, "should have a line break2" },
            ErrorInfo{ ErrorCode::indent_error, 418033, "indent error" },


            // value
            ErrorInfo{ ErrorCode::expect_end_parenthesis, 418133, "expect_end_parenthesis" },

            // string
            ErrorInfo{ ErrorCode::missing_closing_quote, 989800, "missing closing quote" },
            ErrorInfo{ ErrorCode::missing_closing_quote2, 989900, "missing closing quote" },

            ErrorInfo{ ErrorCode::missing_object_delemeter, 7677812, "missing object delimeter"},

            // class
            ErrorInfo{ ErrorCode::invalid_class_name, 7777413, "Invalid class name"},
            ErrorInfo{ ErrorCode::no_brace_for_class, 7777414, "no brace for class"},
            ErrorInfo{ ErrorCode::no_brace_of_end_for_class, 7777415, "no brace of end for class"},

            // fn
            ErrorInfo{ ErrorCode::invalid_fn_name, 7777815, "invalid fn name"},
            ErrorInfo{ ErrorCode::expect_bracket_for_fn_body, 7777816, "expect_bracket_for_fn_body"},
            ErrorInfo{ ErrorCode::expect_parenthesis_for_fn_params, 7777817, "expect '(' for fn parameters"},
            ErrorInfo{ ErrorCode::expect_end_parenthesis_for_fn_params, 7777818, "expect ')' for fn parameters"},

            // return
            ErrorInfo{ ErrorCode::no_value_for_return, 7778818, "no_value_for_return"},

            //----------------------------------------------------------------------------------
            //
            //                                  Logical Errors
            //
            //----------------------------------------------------------------------------------
            ErrorInfo{ErrorCode::no_logical_error, 57770000, "no_logical_error"},
            ErrorInfo{ErrorCode::no_variable_defined, 57770001, "no_variable_defined"},



            ErrorInfo{ ErrorCode::last_keeper, 99999999, "end" },
        };


        static_assert(errorListSize == (sizeof tempList) / sizeof(ErrorInfo), "error list should have the same length");
        //static_assert(is_sorted(tempList), "error List should be sorted"); // C++14


        constexpr int len = (sizeof tempList) / (sizeof tempList[0]);
        static ErrorInfo *sortErrorInfoList[errorListSize];
        for (int i = 0; i < len; i++) {
            auto &&errorInfo = tempList[i];
            if (static_cast<int>(errorInfo.errorIndex) != i) {
                // printf("error info index\n");
                exit(8943210);
            }

            ErrorInfo::ErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = errorInfo;
            sortErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = &errorInfo;
        }

        // check duplicate of error code
        //std::sort(sortErrorInfoList, sortErrorInfoList + errorListSize, acompare);
        //qsort(sortErrorInfoList, sizeof(sortErrorInfoList) / sizeof(sortErrorInfoList[0]), sizeof(ErrorInfo), acompare);

        return 0;
    }






    enum class Language {
        en = 8591000,
        jp = 8591001,
    };


    static const char *translateErrorMessage(ErrorCode errorCode, Language lang) {
        return nullptr;
    }

    static int getErrorId(ErrorCode errorCode) {
        if (!ErrorInfo::errorInfoInitialized) {
            initErrorInfoList();
        }

        auto&& errorInfo = ErrorInfo::ErrorInfoList[static_cast<int>(errorCode)];
        return errorInfo.errorCode;
    }


    static const char *getErrorMessage(ErrorCode errorCode) {
        if (!ErrorInfo::errorInfoInitialized) {
            initErrorInfoList();
        }

        const char *mes = nullptr;
        auto&& errorInfo = ErrorInfo::ErrorInfoList[static_cast<int>(errorCode)];
        mes = errorInfo.msg;

        auto *transMess = translateErrorMessage(errorCode, Language::jp);
        if (transMess != nullptr) {
            mes = transMess;
        }

        return mes;
    }



    #define MAX_REASON_LENGTH 1024
    /**
     * Syntax error is allowed only once
     */
    using SyntaxErrorInfo = struct _errorInfo {
        bool hasError{false};

        ErrorCode errorCode;
        char reason[MAX_REASON_LENGTH + 1];
        int reasonLength = 0;

        st_uint charPosition;
        st_uint charPosition2;

        st_uint linePos1;
        st_uint charPos1;
        st_uint linePos2;
        st_uint charPos2;

        int errorId;
        int charEndPosition;

        // 0: "between start and  end"
        // 1: "from start to end of line,"
        int errorDisplayType = 0;

        static const int SYNTAX_ERROR_RETURN = -1;
    };

    using LogicalErrorInfo = struct _logicalErrorInfo {
        bool hasError{false};

        ErrorCode errorCode;
        char reason[MAX_REASON_LENGTH + 1];
        int reasonLength = 0;

        st_uint charPosition;
        st_uint charPosition2;

        st_uint linePos1;
        st_uint charPos1;
        st_uint linePos2;
        st_uint charPos2;

        int errorId;
        int charEndPosition;

        // 0: "between start and  end"
        // 1: "from start to end of line,"
        int errorDisplayType = 0;

        static const int SYNTAX_ERROR_RETURN = -1;
    };

}

