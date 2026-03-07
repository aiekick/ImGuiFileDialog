#include "Test_ImString.h"

#include <ImGuiFileDialog/ImString.h>

using IGFD::ImString;

////////////////////////////////////////////////////////////////////////////
//// Construction //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Construction_Default() {
    ImString s;
    if (!s.empty()) return false;
    if (s.size() != 0) return false;
    if (strcmp(s.c_str(), "") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_CStr() {
    ImString s("hello");
    if (s.empty()) return false;
    if (s.size() != 5) return false;
    if (strcmp(s.c_str(), "hello") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_CStr_Null() {
    ImString s(nullptr);
    if (!s.empty()) return false;
    if (s.size() != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_CStr_Len() {
    ImString s("hello world", 5);
    if (s.size() != 5) return false;
    if (strcmp(s.c_str(), "hello") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_Range() {
    const char* src = "hello world";
    ImString s(src, src + 5);
    if (s.size() != 5) return false;
    if (strcmp(s.c_str(), "hello") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_Copy() {
    ImString a("hello");
    ImString b(a);
    if (b.size() != 5) return false;
    if (strcmp(b.c_str(), "hello") != 0) return false;
    // modifying b should not affect a
    b = "world";
    if (strcmp(a.c_str(), "hello") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Construction_Move() {
    ImString a("hello");
    ImString b(static_cast<ImString&&>(a));
    if (b.size() != 5) return false;
    if (strcmp(b.c_str(), "hello") != 0) return false;
    if (!a.empty()) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Assignment ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Assignment_CStr() {
    ImString s;
    s = "test";
    if (strcmp(s.c_str(), "test") != 0) return false;
    s = "";
    if (!s.empty()) return false;
    s = nullptr;
    if (!s.empty()) return false;
    return true;
}

bool Test_IGFD_ImString_Assignment_Copy() {
    ImString a("hello");
    ImString b;
    b = a;
    if (strcmp(b.c_str(), "hello") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Assignment_Move() {
    ImString a("hello");
    ImString b;
    b = static_cast<ImString&&>(a);
    if (strcmp(b.c_str(), "hello") != 0) return false;
    if (!a.empty()) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Append ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Append_CStr() {
    ImString s("hello");
    s.append(" world");
    if (strcmp(s.c_str(), "hello world") != 0) return false;
    if (s.size() != 11) return false;
    return true;
}

bool Test_IGFD_ImString_Append_ImString() {
    ImString a("hello");
    ImString b(" world");
    a.append(b);
    if (strcmp(a.c_str(), "hello world") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Append_PushBack() {
    ImString s("abc");
    s.push_back('d');
    s.push_back('e');
    if (strcmp(s.c_str(), "abcde") != 0) return false;
    if (s.size() != 5) return false;
    return true;
}

bool Test_IGFD_ImString_Append_OperatorPlusEqual() {
    ImString s("hello");
    s += " ";
    s += ImString("world");
    s += '!';
    if (strcmp(s.c_str(), "hello world!") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Find //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Find_Char() {
    ImString s("hello world");
    if (s.find('w') != 6) return false;
    if (s.find('z') != ImString::npos) return false;
    if (s.find('l', 5) != 9) return false;
    if (s.find('h', 1) != ImString::npos) return false;
    return true;
}

bool Test_IGFD_ImString_Find_CStr() {
    ImString s("hello world");
    if (s.find("world") != 6) return false;
    if (s.find("xyz") != ImString::npos) return false;
    if (s.find("lo", 4) != ImString::npos) return false;
    if (s.find("lo") != 3) return false;
    return true;
}

bool Test_IGFD_ImString_Find_ImString() {
    ImString s("hello world");
    ImString needle("world");
    if (s.find(needle) != 6) return false;
    return true;
}

bool Test_IGFD_ImString_Rfind_Char() {
    ImString s("hello");
    if (s.rfind('l') != 3) return false;
    if (s.rfind('l', 2) != 2) return false;
    if (s.rfind('z') != ImString::npos) return false;
    return true;
}

bool Test_IGFD_ImString_FindLastOf() {
    ImString s("/path/to/file.txt");
    if (s.find_last_of("/\\") != 8) return false;
    if (s.find_last_of(".") != 13) return false;
    if (s.find_last_of("@#") != ImString::npos) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Substr ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Substr() {
    ImString s("hello world");
    ImString sub = s.substr(6);
    if (strcmp(sub.c_str(), "world") != 0) return false;
    sub = s.substr(0, 5);
    if (strcmp(sub.c_str(), "hello") != 0) return false;
    sub = s.substr(20);
    if (!sub.empty()) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Replace ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Replace() {
    ImString s("hello world");
    s.replace(6, 5, "there");
    if (strcmp(s.c_str(), "hello there") != 0) return false;
    // replace with shorter
    s.replace(6, 5, "you");
    if (strcmp(s.c_str(), "hello you") != 0) return false;
    // replace with longer
    s.replace(6, 3, "everyone");
    if (strcmp(s.c_str(), "hello everyone") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Comparison ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Comparison() {
    ImString a("abc");
    ImString b("abc");
    ImString c("def");
    if (!(a == b)) return false;
    if (a != b) return false;
    if (!(a < c)) return false;
    if (!(c > a)) return false;
    if (!(a == "abc")) return false;
    if (!(a != "def")) return false;
    if (!("abc" == a)) return false;
    if (!("def" != a)) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Concatenation /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Concatenation() {
    ImString a("hello");
    ImString b(" world");
    ImString c = a + b;
    if (strcmp(c.c_str(), "hello world") != 0) return false;
    c = a + " there";
    if (strcmp(c.c_str(), "hello there") != 0) return false;
    c = "hey " + b;
    if (strcmp(c.c_str(), "hey  world") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Format ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Setf() {
    ImString s;
    s.setf("hello %s %d", "world", 42);
    if (strcmp(s.c_str(), "hello world 42") != 0) return false;
    return true;
}

bool Test_IGFD_ImString_Appendf() {
    ImString s("count: ");
    s.appendf("%d", 42);
    if (strcmp(s.c_str(), "count: 42") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// UTF-8 /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_UTF8() {
    // UTF-8 multi-byte characters
    ImString s("\xC3\xA9\xC3\xA8");  // "ee" in UTF-8
    if (s.size() != 4) return false;  // 4 bytes, 2 characters
    if (s.empty()) return false;
    // find works at byte level
    if (s.find("\xC3\xA8") != 2) return false;
    // substr at byte level
    ImString sub = s.substr(0, 2);
    if (sub.size() != 2) return false;
    if (strcmp(sub.c_str(), "\xC3\xA9") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// MemcpySafety //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_MemcpySafe() {
    // Test that ImString is safe to memcpy (for ImVector compatibility)
    ImString a("hello world this is a longer string to force heap allocation");
    char buf[sizeof(ImString)];
    memcpy(buf, &a, sizeof(ImString));
    ImString& b = *(ImString*)buf;
    if (strcmp(b.c_str(), "hello world this is a longer string to force heap allocation") != 0) return false;
    // Zero out a's internals to prevent double-free (simulate what ImVector realloc does)
    memset(&a, 0, sizeof(ImString));
    // b should still be valid
    if (strcmp(b.c_str(), "hello world this is a longer string to force heap allocation") != 0) return false;
    // Clean up b properly
    b.~ImString();
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Clear /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_IGFD_ImString_Clear() {
    ImString s("hello");
    s.clear();
    if (!s.empty()) return false;
    if (s.size() != 0) return false;
    if (strcmp(s.c_str(), "") != 0) return false;
    // can still use after clear
    s = "world";
    if (strcmp(s.c_str(), "world") != 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// Dispatcher ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IF_TEST(name) if (vTest == #name) return name()

bool Test_ImString(const std::string& vTest) {
    IF_TEST(Test_IGFD_ImString_Construction_Default);
    IF_TEST(Test_IGFD_ImString_Construction_CStr);
    IF_TEST(Test_IGFD_ImString_Construction_CStr_Null);
    IF_TEST(Test_IGFD_ImString_Construction_CStr_Len);
    IF_TEST(Test_IGFD_ImString_Construction_Range);
    IF_TEST(Test_IGFD_ImString_Construction_Copy);
    IF_TEST(Test_IGFD_ImString_Construction_Move);
    IF_TEST(Test_IGFD_ImString_Assignment_CStr);
    IF_TEST(Test_IGFD_ImString_Assignment_Copy);
    IF_TEST(Test_IGFD_ImString_Assignment_Move);
    IF_TEST(Test_IGFD_ImString_Append_CStr);
    IF_TEST(Test_IGFD_ImString_Append_ImString);
    IF_TEST(Test_IGFD_ImString_Append_PushBack);
    IF_TEST(Test_IGFD_ImString_Append_OperatorPlusEqual);
    IF_TEST(Test_IGFD_ImString_Find_Char);
    IF_TEST(Test_IGFD_ImString_Find_CStr);
    IF_TEST(Test_IGFD_ImString_Find_ImString);
    IF_TEST(Test_IGFD_ImString_Rfind_Char);
    IF_TEST(Test_IGFD_ImString_FindLastOf);
    IF_TEST(Test_IGFD_ImString_Substr);
    IF_TEST(Test_IGFD_ImString_Replace);
    IF_TEST(Test_IGFD_ImString_Comparison);
    IF_TEST(Test_IGFD_ImString_Concatenation);
    IF_TEST(Test_IGFD_ImString_Setf);
    IF_TEST(Test_IGFD_ImString_Appendf);
    IF_TEST(Test_IGFD_ImString_UTF8);
    IF_TEST(Test_IGFD_ImString_MemcpySafe);
    IF_TEST(Test_IGFD_ImString_Clear);
    return true;
}

#undef IF_TEST
