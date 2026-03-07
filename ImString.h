#pragma once

// IGFD::ImString - Lightweight string class for ImGuiFileDialog
// Based on ImGui conventions: heap-only, memcpy-safe, uses IM_ALLOC/IM_FREE
// UTF-8 byte-level operations (no character-level assumptions)

#include <imgui.h>
#include <imgui_internal.h>
#include <string.h>  // memcpy, memmove, memchr, strlen, strcmp, strstr, strncmp
#include <stdio.h>   // vsnprintf
#include <stdarg.h>  // va_list

namespace IGFD {

class ImString {
    char* m_data{nullptr};
    int m_size{};    // byte length excluding null terminator
    int m_capacity{};  // allocated bytes excluding null terminator (0 = not owned)

    static char* s_EmptyBuf() { static char buf[1] = {'\0'}; return buf; }

    void m_Grow(int needed_capacity) {
        int new_cap = m_capacity ? (m_capacity + m_capacity / 2) : 32;
        if (new_cap < needed_capacity) new_cap = needed_capacity;
        char* new_data = (char*)IM_ALLOC((size_t)new_cap + 1);
        if (m_data && m_size > 0) memcpy(new_data, m_data, (size_t)m_size);
        new_data[m_size] = '\0';
        if (m_capacity > 0) IM_FREE(m_data);
        m_data = new_data;
        m_capacity = new_cap;
    }

public:
    static const int npos = -1;

    // Construction
    ImString() : m_data(s_EmptyBuf()) {}

    ImString(const char* s) : m_data(s_EmptyBuf()) {
        if (s && s[0]) set(s);
    }

    ImString(const char* s, int len) : m_data(s_EmptyBuf()) {
        if (s && len > 0) set(s, len);
    }

    ImString(const char* s, const char* s_end) : m_data(s_EmptyBuf()) {
        if (s && s_end > s) set(s, (int)(s_end - s));
    }

    ImString(const ImString& rhs) : m_data(s_EmptyBuf()), m_size(0), m_capacity(0) {
        if (rhs.m_size > 0) set(rhs.m_data, rhs.m_size);
    }

    ImString(ImString&& rhs) noexcept : m_data(rhs.m_data), m_size(rhs.m_size), m_capacity(rhs.m_capacity) {
        rhs.m_data = s_EmptyBuf();
        rhs.m_size = 0;
        rhs.m_capacity = 0;
    }

    ~ImString() {
        if (m_capacity > 0) IM_FREE(m_data);
    }

    // Assignment
    ImString& operator=(const char* rhs) {
        if (rhs && rhs[0]) set(rhs); else clear();
        return *this;
    }

    ImString& operator=(const ImString& rhs) {
        if (this != &rhs) { if (rhs.m_size > 0) set(rhs.m_data, rhs.m_size); else clear(); }
        return *this;
    }

    ImString& operator=(ImString&& rhs) noexcept {
        if (this != &rhs) {
            if (m_capacity > 0) IM_FREE(m_data);
            m_data = rhs.m_data; m_size = rhs.m_size; m_capacity = rhs.m_capacity;
            rhs.m_data = s_EmptyBuf(); rhs.m_size = 0; rhs.m_capacity = 0;
        }
        return *this;
    }

    // Access
    const char* c_str() const { return m_data; }
    char* c_str() { return m_data; }
    bool empty() const { return m_size == 0; }
    int size() const { return m_size; }
    int length() const { return m_size; }
    int capacity() const { return m_capacity; }
    char operator[](int i) const { IM_ASSERT(i >= 0 && i <= m_size); return m_data[i]; }
    char& operator[](int i) { IM_ASSERT(i >= 0 && i <= m_size); return m_data[i]; }
    const char* begin() const { return m_data; }
    const char* end() const { return m_data + m_size; }

    // Modification
    void clear() {
        if (m_capacity > 0) { m_data[0] = '\0'; m_size = 0; }
        else { m_data = s_EmptyBuf(); m_size = 0; }
    }

    void reserve(int new_capacity) {
        if (new_capacity > m_capacity) m_Grow(new_capacity);
    }

    void set(const char* s) {
        if (s) set(s, (int)strlen(s)); else clear();
    }

    void set(const char* s, int len) {
        if (!s || len <= 0) { clear(); return; }
        if (len > m_capacity) m_Grow(len);
        memmove(m_data, s, (size_t)len);  // memmove for overlapping (substr assign)
        m_data[len] = '\0';
        m_size = len;
    }

    // Append
    void append(const char* s) {
        if (s && s[0]) append(s, (int)strlen(s));
    }

    void append(const char* s, int len) {
        if (!s || len <= 0) return;
        int new_size = m_size + len;
        if (new_size > m_capacity) m_Grow(new_size);
        memcpy(m_data + m_size, s, (size_t)len);
        m_size = new_size;
        m_data[m_size] = '\0';
    }

    void append(const ImString& s) { append(s.m_data, s.m_size); }

    void push_back(char c) {
        if (m_size + 1 > m_capacity) m_Grow(m_size + 1);
        m_data[m_size++] = c;
        m_data[m_size] = '\0';
    }

    ImString& operator+=(const char* rhs) { append(rhs); return *this; }
    ImString& operator+=(const ImString& rhs) { append(rhs); return *this; }
    ImString& operator+=(char c) { push_back(c); return *this; }

    // Search
    int find(char c, int pos = 0) const {
        if (pos < 0 || pos >= m_size) return npos;
        const char* p = (const char*)memchr(m_data + pos, c, (size_t)(m_size - pos));
        return p ? (int)(p - m_data) : npos;
    }

    int find(const char* s, int pos = 0) const {
        if (!s || !s[0] || pos < 0 || pos >= m_size) return npos;
        const char* p = strstr(m_data + pos, s);
        return p ? (int)(p - m_data) : npos;
    }

    int find(const ImString& s, int pos = 0) const { return find(s.m_data, pos); }

    int rfind(char c, int pos = npos) const {
        if (m_size == 0) return npos;
        int start = (pos == npos || pos >= m_size) ? m_size - 1 : pos;
        for (int i = start; i >= 0; --i)
            if (m_data[i] == c) return i;
        return npos;
    }

    int find_last_of(const char* chars, int pos = npos) const {
        if (!chars || m_size == 0) return npos;
        int start = (pos == npos || pos >= m_size) ? m_size - 1 : pos;
        for (int i = start; i >= 0; --i)
            if (strchr(chars, m_data[i])) return i;
        return npos;
    }

    // Substring
    ImString substr(int pos, int len = npos) const {
        if (pos < 0 || pos >= m_size) return ImString();
        int actual_len = (len == npos || pos + len > m_size) ? m_size - pos : len;
        return ImString(m_data + pos, actual_len);
    }

    // Replace
    void replace(int pos, int len, const char* s) {
        if (pos < 0 || pos > m_size) return;
        if (pos + len > m_size) len = m_size - pos;
        int s_len = s ? (int)strlen(s) : 0;
        int new_size = m_size - len + s_len;
        if (new_size > m_capacity) m_Grow(new_size);
        if (s_len != len)
            memmove(m_data + pos + s_len, m_data + pos + len, (size_t)(m_size - pos - len));
        if (s_len > 0) memcpy(m_data + pos, s, (size_t)s_len);
        m_size = new_size;
        m_data[m_size] = '\0';
    }

    void replace(int pos, int len, const ImString& s) { replace(pos, len, s.c_str()); }

    // Comparison
    bool operator==(const char* rhs) const { return strcmp(m_data, rhs ? rhs : "") == 0; }
    bool operator==(const ImString& rhs) const { return m_size == rhs.m_size && strcmp(m_data, rhs.m_data) == 0; }
    bool operator!=(const char* rhs) const { return !(*this == rhs); }
    bool operator!=(const ImString& rhs) const { return !(*this == rhs); }
    bool operator<(const ImString& rhs) const { return strcmp(m_data, rhs.m_data) < 0; }
    bool operator<(const char* rhs) const { return strcmp(m_data, rhs ? rhs : "") < 0; }
    bool operator>(const ImString& rhs) const { return strcmp(m_data, rhs.m_data) > 0; }
    bool operator<=(const ImString& rhs) const { return strcmp(m_data, rhs.m_data) <= 0; }
    bool operator>=(const ImString& rhs) const { return strcmp(m_data, rhs.m_data) >= 0; }

    // Format (printf-style)
    int setf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int r = setfv(fmt, args);
        va_end(args);
        return r;
    }

    int setfv(const char* fmt, va_list args) {
        va_list args2;
        va_copy(args2, args);
        int len = ImFormatStringV(NULL, 0, fmt, args2);
        va_end(args2);
        if (len <= 0) { clear(); return 0; }
        if (len > m_capacity) m_Grow(len);
        m_size = ImFormatStringV(m_data, (size_t)len + 1, fmt, args);
        return m_size;
    }

    int appendf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int r = appendfv(fmt, args);
        va_end(args);
        return r;
    }

    int appendfv(const char* fmt, va_list args) {
        va_list args2;
        va_copy(args2, args);
        int len = ImFormatStringV(NULL, 0, fmt, args2);
        va_end(args2);
        if (len <= 0) return 0;
        int new_size = m_size + len;
        if (new_size > m_capacity) m_Grow(new_size);
        ImFormatStringV(m_data + m_size, (size_t)len + 1, fmt, args);
        m_size = new_size;
        return len;
    }

    // Concatenation operators
    friend ImString operator+(const ImString& lhs, const ImString& rhs) {
        ImString r;
        r.reserve(lhs.m_size + rhs.m_size);
        r.append(lhs);
        r.append(rhs);
        return r;
    }
    friend ImString operator+(const ImString& lhs, const char* rhs) {
        ImString r;
        int rhs_len = rhs ? (int)strlen(rhs) : 0;
        r.reserve(lhs.m_size + rhs_len);
        r.append(lhs);
        r.append(rhs, rhs_len);
        return r;
    }
    friend ImString operator+(const char* lhs, const ImString& rhs) {
        ImString r;
        int lhs_len = lhs ? (int)strlen(lhs) : 0;
        r.reserve(lhs_len + rhs.m_size);
        r.append(lhs, lhs_len);
        r.append(rhs);
        return r;
    }

    // Reverse comparison for const char* == ImString
    friend bool operator==(const char* lhs, const ImString& rhs) { return rhs == lhs; }
    friend bool operator!=(const char* lhs, const ImString& rhs) { return rhs != lhs; }
};

}  // namespace IGFD

// std::hash specialization for use as map/set key
#include <functional>
namespace std {
template <>
struct hash<IGFD::ImString> {
    size_t operator()(const IGFD::ImString& s) const {
        return (size_t)ImHashStr(s.c_str(), (size_t)s.size());
    }
};
}  // namespace std

