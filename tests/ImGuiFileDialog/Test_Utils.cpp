#include "Test_FileStyle.h"

#include <cassert>
#include <cmath>
#include <array>
#include <algorithm>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

using IGFD::ImString;

////////////////////////////////////////////////////////////////////////////
//// ReplaceString /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_ReplaceString_0() {
    ImString code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "FRAG", "TOTO") == false) return false;
    if (code != "@TOTOMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_1() {
    ImString code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "A", "TOTO") == false) return false;
    if (code != "@FRTOTOGMENT TOTO VTOTO TOTOU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_2() {
    ImString code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "Y", "TOTO") == true) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_3() {
    ImString code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 0) == false) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\n\nfloat f = 0.0;\n\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_4() {
    ImString code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "\\", "\n\n") == true) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

// test presence of infinite loop
bool Test_IGFD_Utils_ReplaceString_5() {
    ImString code = "//////";
    if (IGFD::Utils::ReplaceString(code, "/", "/") == true) return false;
    return true;
}

// test recursion growing
bool Test_IGFD_Utils_ReplaceString_6() {
    ImString code = "\n\n\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 0) == false) return false;
    if (code != "\n\n\n\n\n\n") return false;
    code = "\n\n\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 1) == false) return false;
    if (code != "\n\n\n\n\n\n\n\n\n\n\n\n") return false;
    return true;
}

// test recursion reducing
bool Test_IGFD_Utils_ReplaceString_7() {
    ImString code = "//////";
    if (IGFD::Utils::ReplaceString(code, "//", "/", 0) == false) return false;
    if (code != "///") return false;
    code = "//////";
    if (IGFD::Utils::ReplaceString(code, "//", "/", 100) == false) return false;
    if (code != "/") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// SplitStringToVector // Delimiter char /////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_0() {
    ImString code = "TOTO;VA;AU;ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), ';', false);
    if (arr.Size != 4) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_1() {
    ImString code = "TOTO;VA;AU;;ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), ';', false);
    if (arr.Size != 4) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_2() {
    ImString code = "TOTO;VA;AU;;ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), ';', true);
    if (arr.Size != 5) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "") return false;
    if (arr[4] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_3() {
    ImString code = "TOTO:VA;AU;;ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), ';', false);
    if (arr.Size != 3) return false;
    if (arr[0] != "TOTO:VA") return false;
    if (arr[1] != "AU") return false;
    if (arr[2] != "ZOO") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// SplitStringToVector // Delimiter std::string //////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_0() {
    ImString code = "TOTO##VA##AU##ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), "##", false);
    if (arr.Size != 4) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_1() {
    ImString code = "TOTO##VA##AU####ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), "##", false);
    if (arr.Size != 4) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_2() {
    ImString code = "TOTO##VA##AU####ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), "##", true);
    if (arr.Size != 5) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "") return false;
    if (arr[4] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_3() {
    ImString code = "TOTO:VA##AU####ZOO";
    ImVector<ImString> arr;
    IGFD::Utils::SplitStringToVector(arr, code.c_str(), "##", false);
    if (arr.Size != 3) return false;
    if (arr[0] != "TOTO:VA") return false;
    if (arr[1] != "AU") return false;
    if (arr[2] != "ZOO") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// LowerCaseString ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_LowerCaseString_0() {
    auto res = IGFD::Utils::LowerCaseString("Hello WORLD 123!");
    if (res != "hello world 123!") return false;
    res = IGFD::Utils::LowerCaseString("");
    if (res != "") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// GetCharCountInString //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_GetCharCountInString_0() {
    if (IGFD::Utils::GetCharCountInString("hello.world.txt", '.') != 2U) return false;
    if (IGFD::Utils::GetCharCountInString("nochar", '.') != 0U) return false;
    if (IGFD::Utils::GetCharCountInString("", '.') != 0U) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// GetLastCharPosWithMinCharCount ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_GetLastCharPosWithMinCharCount_0() {
    // "a.b.c.d" has dots at pos 1, 3, 5
    // minCharCount=1 => last 1 dot from end => pos 5
    auto pos = IGFD::Utils::GetLastCharPosWithMinCharCount("a.b.c.d", '.', 1);
    if (pos != 5U) return false;
    // minCharCount=2 => last 2 dots from end => pos 3
    pos = IGFD::Utils::GetLastCharPosWithMinCharCount("a.b.c.d", '.', 2);
    if (pos != 3U) return false;
    // minCharCount=3 => last 3 dots from end => pos 1
    pos = IGFD::Utils::GetLastCharPosWithMinCharCount("a.b.c.d", '.', 3);
    if (pos != 1U) return false;
    // minCharCount=0 => returns npos
    pos = IGFD::Utils::GetLastCharPosWithMinCharCount("a.b.c.d", '.', 0);
    if (pos != (size_t)-1) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// FormatFileSize ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_FormatFileSize_0() {
    auto res = IGFD::Utils::FormatFileSize(0);
    if (res.first != "0") return false;
    // 500 bytes
    res = IGFD::Utils::FormatFileSize(500);
    if (res.first != "500") return false;
    // 1.5 KB = 1536 bytes
    res = IGFD::Utils::FormatFileSize(1536);
    if (res.first != "1.50") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// GetPathSeparator //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_GetPathSeparator_0() {
    const char* sep = IGFD::Utils::GetPathSeparator();
#ifdef _WIN32
    if (strcmp(sep, "\\") != 0) return false;
#else
    if (strcmp(sep, "/") != 0) return false;
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Natural Sorting related ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

namespace IGFD {
class TestUtils {
public:
    static bool ExtractNumFromStringAtPos(const char* str, size_t& pos, double& vOutNum) {
        return Utils::M_ExtractNumFromStringAtPos(str, str ? strlen(str) : 0, pos, vOutNum);
    }
};
}  // namespace IGFD

    // invalids numbers
bool Test_IGFD_Utils_ExtractNumFromStringAtPos_0() {
    double n = 0.0;
    size_t p = 0;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("++2.5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("--2.5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("..2.5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("+-+2.5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("-+2.5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("bleed", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("bled", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("e1e", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("+", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("-", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("e", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos(".", p = 0, n)) return false;

    // supported by strtod but will slow down more andwhy sorting by that
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("INF", p = 0, n)) return false; 
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("NAN", p = 0, n)) return false;
    return true;
}

// valid numbers
bool Test_IGFD_Utils_ExtractNumFromStringAtPos_1() {
    double n = 0.0;
    size_t p = 0;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("+2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("-2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos(".2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("-2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("+2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("1e-5", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("1e", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("1e-", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("1p", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("1p-", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("0x", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("0xABCDEF", p = 0, n)) return false;
    if (IGFD::TestUtils::ExtractNumFromStringAtPos("0xabcdef", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("1E32.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("0x14", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("0X14", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("2p2.5", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("2.5P-2.9", p = 0, n)) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("4588E4588", p = 0, n)) return false;
    return true;
}

// valids cases, verify number and new pos
bool Test_IGFD_Utils_ExtractNumFromStringAtPos_2() {
    double n = 0.0;
    size_t p = 0;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("abc-2.8abc", p = 3, n)) return false;
    if (n != -2.8) return false;
    if (p != 7U) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("abc+8.9abc", p = 3, n)) return false;
    if (n != 8.9) return false;
    if (p != 7U) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("abc10.6546abc", p = 3, n)) return false;
    if (n != 10.6546) return false;
    if (p != 10U) return false;
    if (!IGFD::TestUtils::ExtractNumFromStringAtPos("4588E4588", p = 3, n)) return false;
    if (!std::isinf(n)) return false;
    if (p != 9U) return false;
    return true;
}

// natural sort : INSENSITIVE CASE, ASCENDING
bool Test_IGFD_Utils_NaturalCompare_0() {
    static constexpr size_t s_count_items           = 16U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "abc1e5",       //
            "abc0.5abc",    //
            "abc-1.9",      //
            "tot4",         //
            "abc+2.5",      //
            "TOT30",        //
            "TOT40",        //
            "tot1.5",       //
            "abs1.5z2.6",   //
            "tot1.",        //
            "TOT2",         //
            "tot22",        //
            "abc1e-5",      //
            "abs1.5z2.5",   //
            "abs1.5z-2.2",  //
            "tot3",         //
        };

    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), true, false);
              });

    std::array<std::string, s_count_items> good_sort =  //
        {
            "abc-1.9",      //
            "abc1e-5",      //
            "abc0.5abc",    //
            "abc+2.5",      //
            "abc1e5",       //
            "abs1.5z-2.2",  //
            "abs1.5z2.5",   //
            "abs1.5z2.6",   //
            "tot1.",        //
            "tot1.5",       //
            "TOT2",         //
            "tot3",         //
            "tot4",         //
            "tot22",        //
            "TOT30",        //
            "TOT40",        //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

// natural sort : INSENSITIVE CASE, DESCENDING
bool Test_IGFD_Utils_NaturalCompare_1() {
    static constexpr size_t s_count_items           = 16U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "abc1e5",       //
            "abc0.5abc",    //
            "abc-1.9",      //
            "tot4",         //
            "abc+2.5",      //
            "TOT30",        //
            "TOT40",        //
            "tot1.5",       //
            "abs1.5z2.6",   //
            "tot1.",        //
            "TOT2",         //
            "tot22",        //
            "abc1e-5",      //
            "abs1.5z2.5",   //
            "abs1.5z-2.2",  //
            "tot3",         //
        };

    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), true, true);
              });

    std::array<std::string, s_count_items> good_sort =  //
        {
            "TOT40",  //
            "TOT30",  //
            "tot22",  //
            "tot4",   //
            "tot3",   //
            "TOT2",   //
            "tot1.5",  //
            "tot1.",   //
            "abs1.5z2.6",  //
            "abs1.5z2.5",  //
            "abs1.5z-2.2",  //
            "abc1e5",       //
            "abc+2.5",      //
            "abc0.5abc",    //
            "abc1e-5",      //
            "abc-1.9",      //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

// natural sort : SENSITIVE CASE, ASCENDING
bool Test_IGFD_Utils_NaturalCompare_2() {
    static constexpr size_t s_count_items           = 16U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "abc1e5",       //
            "abc0.5abc",    //
            "abc-1.9",      //
            "tot4",         //
            "abc+2.5",      //
            "TOT30",        //
            "TOT40",        //
            "tot1.5",       //
            "abs1.5z2.6",   //
            "tot1.",        //
            "TOT2",         //
            "tot22",        //
            "abc1e-5",      //
            "abs1.5z2.5",   //
            "abs1.5z-2.2",  //
            "tot3",         //
        };

    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), false, false);
              });

    std::array<std::string, s_count_items> good_sort =  //
        {
            "TOT2",         //
            "TOT30",        //
            "TOT40",        //
            "abc-1.9",      //
            "abc1e-5",      //
            "abc0.5abc",    //
            "abc+2.5",      //
            "abc1e5",       //
            "abs1.5z-2.2",  //
            "abs1.5z2.5",   //
            "abs1.5z2.6",   //
            "tot1.",        //
            "tot1.5",       //
            "tot3",         //
            "tot4",         //
            "tot22",        //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

// natural sort : SENSITIVE CASE, DESCENDING
bool Test_IGFD_Utils_NaturalCompare_3() {
    static constexpr size_t s_count_items           = 16U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "abc1e5",       //
            "abc0.5abc",    //
            "abc-1.9",      //
            "tot4",         //
            "abc+2.5",      //
            "TOT30",        //
            "TOT40",        //
            "tot1.5",       //
            "abs1.5z2.6",   //
            "tot1.",        //
            "TOT2",         //
            "tot22",        //
            "abc1e-5",      //
            "abs1.5z2.5",   //
            "abs1.5z-2.2",  //
            "tot3",         //
        };

    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), false, true);
              });

    std::array<std::string, s_count_items> good_sort =  //
        {
            "tot22",        //
            "tot4",         //
            "tot3",         //
            "tot1.5",       //
            "tot1.",        //
            "abs1.5z2.6",   //
            "abs1.5z2.5",   //
            "abs1.5z-2.2",  //
            "abc1e5",       //
            "abc+2.5",      //
            "abc0.5abc",    //
            "abc1e-5",      //
            "abc-1.9",      //
            "TOT40",        //
            "TOT30",        //
            "TOT2",         //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

// natural sort : a crash occured is std::sort where a is filedialog_1 and b is filedialog
bool Test_IGFD_Utils_NaturalCompare_4() {
    static constexpr size_t s_count_items           = 5U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "filed",         //
            "file",          //
            "filedialog_1",  //
            "filedialog",    //
            "filedialog_2",  //
        };

    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), true, false);
              });

    std::array<std::string, s_count_items> good_sort =  //
        {
            "file",          //
            "filed",         //
            "filedialog",    //
            "filedialog_1",  //
            "filedialog_2",  //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

// natural sort : not good sort in descending but ok ins ascending
bool Test_IGFD_Utils_NaturalCompare_5() {
    static constexpr size_t s_count_items           = 3U;
    std::array<std::string, s_count_items> bad_sort =  //
        {
            "test2.txt",   //
            "test1.txt",   //
            "test11.txt",  //
        };

    // ASCENDING
    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), true, false);
              });

    std::array<std::string, s_count_items> good_sort_asc =  //
        {
            "test1.txt",   //
            "test2.txt",   //
            "test11.txt",  //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort_asc.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }

    // DESCENDING
    std::sort(bad_sort.begin(), bad_sort.end(),                         //
              [](const std::string& a, const std::string& b) -> bool {  //
                  return IGFD::Utils::NaturalCompare(a.c_str(), b.c_str(), true, true);
              });

    std::array<std::string, s_count_items> good_sort_desc =  //
        {
            "test11.txt",  //
            "test2.txt",   //
            "test1.txt",   //
        };

    for (size_t idx = 0; idx < s_count_items; ++idx) {
        if (good_sort_desc.at(idx) != bad_sort.at(idx)) {
            return false;
        }
    }
    return true;
};

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v) \
    if (vTest == std::string(#v)) return v()

bool Test_Utils(const std::string& vTest) {
    IfTestExist(Test_IGFD_Utils_ReplaceString_0);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_1);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_2);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_3);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_4);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_5);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_6);
    else IfTestExist(Test_IGFD_Utils_ReplaceString_7);

    IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_char_0);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_char_1);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_char_2);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_char_3);

    IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_0);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_1);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_2);
    else IfTestExist(Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_3);

    IfTestExist(Test_IGFD_Utils_LowerCaseString_0);
    else IfTestExist(Test_IGFD_Utils_GetCharCountInString_0);
    else IfTestExist(Test_IGFD_Utils_GetLastCharPosWithMinCharCount_0);
    else IfTestExist(Test_IGFD_Utils_FormatFileSize_0);
    else IfTestExist(Test_IGFD_Utils_GetPathSeparator_0);

    IfTestExist(Test_IGFD_Utils_ExtractNumFromStringAtPos_0);
    else IfTestExist(Test_IGFD_Utils_ExtractNumFromStringAtPos_1);
    else IfTestExist(Test_IGFD_Utils_ExtractNumFromStringAtPos_2);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_0);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_1);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_2);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_3);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_4);
    else IfTestExist(Test_IGFD_Utils_NaturalCompare_5);

    assert(0);

    return false;
}

#pragma endregion
