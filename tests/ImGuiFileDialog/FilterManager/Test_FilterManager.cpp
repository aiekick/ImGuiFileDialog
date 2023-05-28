#include "Test_FilterManager.h"

#include <cassert>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

////////////////////////////////////////////////////////////////////////////
//// Filter use cases //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// ".*,.cpp,.h,.hpp" => simple filters
// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters
// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex
// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // CollectionFilters /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

IGFD::FilterManager Test_ParseFilters(const char* vFilters) {
    IGFD::FilterManager mgr;

    std::vector<IGFD::FilterManager::FilterInfos> prParsedFilters;

#if 0
	mgr.ParseFilters(vFilters);
#else
    std::string filter_string = vFilters;

    IGFD::FilterManager::FilterInfos infos;

    bool start_collection = false;
    bool end_collection = false;
    bool start_regex = false;
    bool end_regex = false;

    std::string word;

    char last_char_c = 0;
    for (char c : filter_string) {
        if (c == '{') {
            last_char_c = c;
            if (start_regex) {
                word += c;
            } else {
                if (!word.empty()) {
                    infos.filter = word;
                    prParsedFilters.push_back(infos);
                }
                word.clear();
                infos.clear();
                start_collection = true;
                end_collection = false;
            }
        } else if (c == '}') {
            last_char_c = c;
            if (start_regex) {
                word += c;
            } else {
                if (start_collection) {
                    if (prParsedFilters.empty()) { 
                        prParsedFilters.emplace_back();
                    }
                    prParsedFilters.back().collectionfilters.emplace(word);
                    prParsedFilters.back().collectionfilters_optimized.emplace(IGFD::Utils::LowerCaseString(word));

                    start_collection = false;
                    end_collection = true;
                } else {
                    if (prParsedFilters.size() > 1U) {
                        prParsedFilters.erase(--prParsedFilters.end());
                    } else {
                        prParsedFilters.clear();
                    }
                }
                word.clear();
            }
        } else if (c == '(') {
            start_regex = true;
            end_regex = false;
            last_char_c = c;
            word += c;
        } else if (c == ')') {
            last_char_c = c;
            word += c;
            if (start_regex) {
                if (start_collection) {
                    try {
                        prParsedFilters.back().collectionfilters_regex.emplace_back(word);  // this can fail so is first
                        prParsedFilters.back().collectionfilters.emplace(word);
                        prParsedFilters.back().collectionfilters_optimized.emplace(IGFD::Utils::LowerCaseString(word));
                    } catch (std::exception&) {}
                } else {
                    if (!word.empty()) {
                        try {
                            infos.filter_regex = std::regex(word); // this can fail so is first
                            infos.filter = word;
                            infos.filter_optimized = IGFD::Utils::LowerCaseString(infos.filter);
                            prParsedFilters.push_back(infos);
                        } catch (std::exception&) {}
                    }
                }
                word.clear();
                infos.clear();
                start_regex = false;
                end_regex = true;
            } else if (!start_collection) {
                if (prParsedFilters.size() > 1U) {
                    prParsedFilters.erase(--prParsedFilters.end());
                } else {
                    prParsedFilters.clear();
                }
            } else {
                word.clear();
            }
        } else if (c == '.') {
            last_char_c = c;
            word += c;
        } else if (c == ',') {
            if (!start_regex) {
                if (word.size() > 1U && word[0] == '.') {
                    if (start_collection) {
                        if (prParsedFilters.empty()) { prParsedFilters.emplace_back(); }
                        prParsedFilters.back().collectionfilters.emplace(word);
                        prParsedFilters.back().collectionfilters_optimized.emplace(IGFD::Utils::LowerCaseString(word));
                    } else {
                        infos.filter = word;
                        prParsedFilters.push_back(infos);
                        infos.clear();
                    }
                }
            }
            start_regex = false;
            end_regex = true;
            word.clear();
        } else {
            word += c;
        }
    }

    if (start_collection) {
        if (last_char_c != '}') {
            if (prParsedFilters.size() > 1U) {
                prParsedFilters.erase(--prParsedFilters.end());
            } else {
                prParsedFilters.clear();
            }
        }
    } else if (last_char_c != '}') {
        if (word.size() > 1U && word[0] == '.') {
            infos.filter = word;
            prParsedFilters.push_back(infos);
        }
    }

    mgr.prParsedFilters = prParsedFilters;
#endif

    return mgr;
}

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Simple Filters ////////////////////////////////////////
//// ".*,.cpp,.h,.hpp" => simple filters ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#pragma region Filters Simple

bool Test_FilterManager_ParseFilters_Filters_Simple_0() {
    auto mgr = Test_ParseFilters(".*");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != ".*") return false;

    return true;
}

bool Test_FilterManager_ParseFilters_Filters_Simple_1() {
    auto mgr = Test_ParseFilters(".*,.cpp,.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != ".*") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// missing '.'
bool Test_FilterManager_ParseFilters_Filters_Simple_2() {
    auto mgr = Test_ParseFilters("hpp");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// empty filter after ','
bool Test_FilterManager_ParseFilters_Filters_Simple_3() {
    auto mgr = Test_ParseFilters(".hpp,");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != ".hpp") return false;

    return true;
}

// good filter
bool Test_FilterManager_ParseFilters_Filters_Simple_4() {
    auto mgr = Test_ParseFilters("..hpp");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;

    return true;
}

// good filter, but the second is empty
bool Test_FilterManager_ParseFilters_Filters_Simple_5() {
    auto mgr = Test_ParseFilters("..hpp,");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;

    return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_6() {
    auto mgr = Test_ParseFilters("..hpp,.hpp");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;

    return true;
}

// good filter, but the third not
bool Test_FilterManager_ParseFilters_Filters_Simple_7() {
    auto mgr = Test_ParseFilters("..hpp,.hpp,.");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;

    return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_8() {
    auto mgr = Test_ParseFilters("..hpp,.hpp,. .");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;
    if (mgr.prParsedFilters[2].filter != ". .") return false;

    return true;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // CollectionFilters /////////////////////////////////////////////////////////////
//// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters //////
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Collection

// must be good {filter0, filter1}
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_0() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// missing end }
// 2022/06/21 => give an infinite loop
bool Test_FilterManager_ParseFilters_Filters_Collection_1() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// missing start }
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_2() {
    auto mgr = Test_ParseFilters("Source Files.cpp,.c}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_3() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx}");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_4() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx},");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_5() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx} , ");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// first { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_6() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files.hpp,.h,.hxx} , ");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_7() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx , .md");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_8() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_9() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md,Other Files{.txt,.doc}");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;
    if (mgr.prParsedFilters[3].filter != "Other Files") return false;
    if (mgr.prParsedFilters[3].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".txt") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".doc") == mgr.prParsedFilters[3].collectionfilters.end()) return false;

    return true;
}

// first { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_10() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files.txt,.doc}");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_11() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files{.txt,.doc");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;

    return true;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // simple filters with regex /////////////////////////////
//// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex /////////////
////////////////////////////////////////////////////////////////////////////

#pragma region Filters Simple Regex

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_0() {
    auto mgr = Test_ParseFilters("([.][0-9]{3})");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_1() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_2() {
    auto mgr = Test_ParseFilters("[.][0-9]{3})");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_3() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_4() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,([.][0-9]{3}),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_5() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,([.][0-9]{3}),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// last ) missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_6() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,([.][0-9]{3},.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// first ( missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_7() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,[.][0-9]{3}),.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_8() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}),.cpp,([.][0-9]{3}) ,.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "([.][0-9]{3})") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Collection filters with regex //////////////////////////////////////////
//// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex /////////////
/////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Collection with Regex

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_0() {
    auto mgr = Test_ParseFilters("frames files{([.][0-9]{3}),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// invalid regex
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_1() {
    auto mgr = Test_ParseFilters("frames files{(.001,.NNN),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// msut be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_2() {
    auto mgr = Test_ParseFilters("frames files(.frames){([.][0-9]{3}),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.frames)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// msut be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_3() {
    auto mgr = Test_ParseFilters("frames files(.cpp,.hpp){([.][0-9]{3}),.cpp, .hpp}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.cpp,.hpp)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".hpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v) vTest == std::string(v)
bool Test_FilterManager(const std::string& vTest) {
    if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::0"))
        return Test_FilterManager_ParseFilters_Filters_Simple_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::1"))
        return Test_FilterManager_ParseFilters_Filters_Simple_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::2"))
        return Test_FilterManager_ParseFilters_Filters_Simple_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::3"))
        return Test_FilterManager_ParseFilters_Filters_Simple_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::4"))
        return Test_FilterManager_ParseFilters_Filters_Simple_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::5"))
        return Test_FilterManager_ParseFilters_Filters_Simple_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::6"))
        return Test_FilterManager_ParseFilters_Filters_Simple_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::7"))
        return Test_FilterManager_ParseFilters_Filters_Simple_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::8"))
        return Test_FilterManager_ParseFilters_Filters_Simple_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::0"))
        return Test_FilterManager_ParseFilters_Filters_Collection_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::1"))
        return Test_FilterManager_ParseFilters_Filters_Collection_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::2"))
        return Test_FilterManager_ParseFilters_Filters_Collection_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::3"))
        return Test_FilterManager_ParseFilters_Filters_Collection_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::4"))
        return Test_FilterManager_ParseFilters_Filters_Collection_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::5"))
        return Test_FilterManager_ParseFilters_Filters_Collection_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::6"))
        return Test_FilterManager_ParseFilters_Filters_Collection_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::7"))
        return Test_FilterManager_ParseFilters_Filters_Collection_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::8"))
        return Test_FilterManager_ParseFilters_Filters_Collection_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::9"))
        return Test_FilterManager_ParseFilters_Filters_Collection_9();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::10"))
        return Test_FilterManager_ParseFilters_Filters_Collection_10();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::11"))
        return Test_FilterManager_ParseFilters_Filters_Collection_11();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::0"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::1"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::2"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::3"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::4"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::5"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::6"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::7"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::8"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::0"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::1"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::2"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::3"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_3();

    assert(0);

    return false;
}