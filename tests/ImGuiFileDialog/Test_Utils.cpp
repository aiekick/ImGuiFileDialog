#include "Test_FileStyle.h"

#include <cassert>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

////////////////////////////////////////////////////////////////////////////
//// ReplaceString /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_ReplaceString_0() {
    std::string code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "FRAG", "TOTO") == false) return false;
    if (code != "@TOTOMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_1() {
    std::string code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "A", "TOTO") == false) return false;
    if (code != "@FRTOTOGMENT TOTO VTOTO TOTOU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_2() {
    std::string code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "Y", "TOTO") == true) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_3() {
    std::string code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 0) == false) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\n\nfloat f = 0.0;\n\n") return false;
    return true;
}

bool Test_IGFD_Utils_ReplaceString_4() {
    std::string code = "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n";
    if (IGFD::Utils::ReplaceString(code, "\\", "\n\n") == true) return false;
    if (code != "@FRAGMENT TOTO VA AU ZOO\nfloat f = 0.0;\n") return false;
    return true;
}

// test presence of infinite loop
bool Test_IGFD_Utils_ReplaceString_5() {
    std::string code = "//////";
    if (IGFD::Utils::ReplaceString(code, "/", "/") == true) return false;
    return true;
}

// test recursion growing
bool Test_IGFD_Utils_ReplaceString_6() {
    std::string code = "\n\n\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 0) == false) return false;
    if (code != "\n\n\n\n\n\n") return false;
    code = "\n\n\n";
    if (IGFD::Utils::ReplaceString(code, "\n", "\n\n", 1) == false) return false;
    if (code != "\n\n\n\n\n\n\n\n\n\n\n\n") return false;
    return true;
}

// test recursion reducing
bool Test_IGFD_Utils_ReplaceString_7() {
    std::string code = "//////";
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
    std::string code = "TOTO;VA;AU;ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, ';', false);
    if (arr.size() != 4U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_1() {
    std::string code = "TOTO;VA;AU;;ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, ';', false);
    if (arr.size() != 4U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_2() {
    std::string code = "TOTO;VA;AU;;ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, ';', true);
    if (arr.size() != 5U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "") return false;
    if (arr[4] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_char_3() {
    std::string code = "TOTO:VA;AU;;ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, ';', false);
    if (arr.size() != 3U) return false;
    if (arr[0] != "TOTO:VA") return false;
    if (arr[1] != "AU") return false;
    if (arr[2] != "ZOO") return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// SplitStringToVector // Delimiter std::string //////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_0() {
    std::string code = "TOTO##VA##AU##ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, "##", false);
    if (arr.size() != 4U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_1() {
    std::string code = "TOTO##VA##AU####ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, "##", false);
    if (arr.size() != 4U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_2() {
    std::string code = "TOTO##VA##AU####ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, "##", true);
    if (arr.size() != 5U) return false;
    if (arr[0] != "TOTO") return false;
    if (arr[1] != "VA") return false;
    if (arr[2] != "AU") return false;
    if (arr[3] != "") return false;
    if (arr[4] != "ZOO") return false;
    return true;
}

bool Test_IGFD_Utils_SplitStringToVector_delimiter_std_string_3() {
    std::string code = "TOTO:VA##AU####ZOO";
    auto arr         = IGFD::Utils::SplitStringToVector(code, "##", false);
    if (arr.size() != 3U) return false;
    if (arr[0] != "TOTO:VA") return false;
    if (arr[1] != "AU") return false;
    if (arr[2] != "ZOO") return false;
    return true;
}


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

    assert(0);

    return false;
}

#pragma endregion
