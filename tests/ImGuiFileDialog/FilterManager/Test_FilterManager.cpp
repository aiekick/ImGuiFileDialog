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
    IGFD::FilterManager::FilterInfos prSelectedFilter;
    std::string puDLGFilters;

#if 0
	mgr.ParseFilters(vFilters);
#else
    //////////////////////////////////////////////////////

    prParsedFilters.clear();

    if (vFilters)
        puDLGFilters = vFilters;  // file mode
    else
        puDLGFilters.clear();  // directory mode

    if (!puDLGFilters.empty()) {

		/* Rules
		0) a filter must have 2 chars mini and the first must be a .
		1) a regex must be in (( and ))
		2) a , will separate filters except if between a ( and )
		3) name{filter1, filter2} is a spetial form for collection filters
		3.1) the name can be composed of what you want except { and }
		3.2) the filter can be a regex
		4) the filters cannot integrate these chars '(' ')' '{' '}' ' ' except for a regex with respect to rule 1) 
        5) the filters cannot integrate a ',' 
		*/

        bool start_collection = false;
        bool end_collection = false;
        bool start_regex = false;
        bool end_regex = false;

        std::string word;
        std::string filter_name;
        size_t idx = 0U;

        char last_char_c = 0;
        for (char c : puDLGFilters) {
            if (c == '{') {
                if (start_regex) {
                    word += c;
                    filter_name += c;
                } else {
                    if (last_char_c == ')') {
                        prParsedFilters.emplace_back();
                        prParsedFilters.back().filter = filter_name;
                    } else if (!word.empty()) {
                        prParsedFilters.emplace_back();
                        prParsedFilters.back().filter = word;
                    }
                    filter_name.clear();
                    word.clear();
                    start_collection = true;
                    end_collection = false;
                }
                last_char_c = c;
            } else if (c == '}') {
                if (start_regex) {
                    word += c;
                    filter_name += c;
                } else {
                    if (start_collection) {
                        if (prParsedFilters.empty()) { prParsedFilters.emplace_back(); }
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
                last_char_c = c;
            } else if (c == '(') {
				if (last_char_c == '(') {
                    start_regex = true;
                    end_regex = false;
                }
                word += c;
                filter_name += c;
                last_char_c = c;
            } else if (c == ')') {
                word += c;
                filter_name += c;
				if (start_regex) {
					if (last_char_c == ')') {
						if (start_collection) {
							try {
								prParsedFilters.back().collectionfilters_regex.emplace_back(word);	// this can fail so is first
								prParsedFilters.back().collectionfilters.emplace(word);
								prParsedFilters.back().collectionfilters_optimized.emplace(IGFD::Utils::LowerCaseString(word));
							} catch (std::exception&) {}
						} else {
							if (!word.empty()) {
								try {
									auto rx = std::regex(word);	 // this can fail so is first
									prParsedFilters.emplace_back();
									prParsedFilters.back().filter_regex		= std::regex(word);	 // this can fail so is first
									prParsedFilters.back().filter			= word;
									prParsedFilters.back().filter_optimized = IGFD::Utils::LowerCaseString(word);
								} catch (std::exception&) {}
							}
						}
						word.clear();
						start_regex = false;
						end_regex	= true;
                    }
				} else if (!start_collection && last_char_c != ')') {
                    if (prParsedFilters.size() > 1U) {
                        prParsedFilters.erase(--prParsedFilters.end());
                    } else {
                        prParsedFilters.clear();
                    }
                } else {
                    word.clear();
                }
                last_char_c = c;
            } else if (c == '.') {
                word += c;
                filter_name += c;
                last_char_c = c;
            } else if (c == ',') {
                filter_name += c;
                if (!start_regex) {
                    if (word.size() > 1U && word[0] == '.') {
                        if (start_collection) {
                            if (prParsedFilters.empty()) { prParsedFilters.emplace_back(); }
                            prParsedFilters.back().collectionfilters.emplace(word);
                            prParsedFilters.back().collectionfilters_optimized.emplace(IGFD::Utils::LowerCaseString(word));
                        } else {
                            prParsedFilters.emplace_back();
                            prParsedFilters.back().filter = word;
                        }
                    }
                }
                start_regex = false;
                end_regex = true;
                word.clear();
            } else {
                word += c;
                filter_name += c;
            }
            ++idx;
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
                prParsedFilters.emplace_back();
                prParsedFilters.back().filter = word;
            }
        }

        bool currentFilterFound = false;

        for (const auto& it : prParsedFilters) {
            if (it.filter == prSelectedFilter.filter) {
                currentFilterFound = true;
                prSelectedFilter = it;
            }
        }

        if (!currentFilterFound) {
            if (!prParsedFilters.empty()) prSelectedFilter = *prParsedFilters.begin();
        }
    }

    //////////////////////////////////////////////////////

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
    auto mgr = Test_ParseFilters("(([.][0-9]{3}))");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_1() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_2() {
    auto mgr = Test_ParseFilters("[.][0-9]{3}))");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_3() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_4() {
	auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_5() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// last ) missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_6() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3},.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// first ( missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_7() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,[.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_8() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})) ,.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// must fail
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_9() {
	auto mgr = Test_ParseFilters("(([.][0-9]{3})");

	if (mgr.prParsedFilters.size() != 0U) return false;

	return true;
}

// must fail
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_10() {
	auto mgr = Test_ParseFilters("([.][0-9]{3}))");

	if (mgr.prParsedFilters.size() != 0U) return false;

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
    auto mgr = Test_ParseFilters("frames files{(([.][0-9]{3})),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// invalid regex
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_1() {
    auto mgr = Test_ParseFilters("frames files{((.001,.NNN)),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_2() {
    auto mgr = Test_ParseFilters("frames files(.frames){(([.][0-9]{3})),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.frames)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_3() {
    auto mgr = Test_ParseFilters("frames files(.cpp,.hpp){(([.][0-9]{3})),.cpp,.hpp}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.cpp,.hpp)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".hpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Divers Tests ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Divers Tests

/* Rules
0) a filter must have 2 chars mini and the first must be a .
1) a regex must be in (( and ))
2) a , will separate filters except if between a ( and )
3) name{filter1, filter2} is a spetial form for collection filters
3.1) the name can be composed of what you want except { and }
3.2) the filter can be a regex
4) the filters cannot integrate these chars '(' ')' '{' '}' except for a regex with respect to rule 1)
5) the filters cannot integrate a ','
6) all spaces in filters will be shrinked, the best you can do is avoid them 
*/

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_0() {
    auto mgr = Test_ParseFilters("\
All files{.*},\
Frames(.png.001,.NNN){(([.][0-9]{3}))},\
Frames(XXX.png){(([0-9]{3}.png))},\
Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},\
Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},\
.md\
Frames(XXX.png){(([0-9]{3}.png))}");

    if (mgr.prParsedFilters.size() != 7U) return false;
    if (mgr.prParsedFilters[0].filter != "All files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".*") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Frames(.001,.NNN)") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != "Source files (*.cpp *.h *.hpp)") return false;
    if (mgr.prParsedFilters[2].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[2].collectionfilters.find(".cpp") == mgr.prParsedFilters[2].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].collectionfilters.find(".h") == mgr.prParsedFilters[2].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].collectionfilters.find(".hpp") == mgr.prParsedFilters[2].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].filter != "Image files (*.png *.gif *.jpg *.jpeg)") return false;
    if (mgr.prParsedFilters[3].collectionfilters.size() != 4U) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".png") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".gif") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".jpg") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".jpeg") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[4].filter != ".md") return false;
	if (mgr.prParsedFilters[4].collectionfilters.size() != 0U) return false;
	if (mgr.prParsedFilters[5].filter != "Frames(XXX.png)") return false;
	if (mgr.prParsedFilters[5].collectionfilters.size() != 1U) return false;
	if (mgr.prParsedFilters[5].collectionfilters.find("([0-9]{3}.png)") == mgr.prParsedFilters[5].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_1() {
    auto mgr = Test_ParseFilters("Regex Custom*.h{(Custom.+[.]h)}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_2() {
    auto mgr = Test_ParseFilters("C++ File (*.cpp){.cpp}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_3() {
    auto mgr = Test_ParseFilters("C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("([.][0-9]{3})") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

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
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::9"))
		return Test_FilterManager_ParseFilters_Filters_Simple_Regex_9();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::10"))
		return Test_FilterManager_ParseFilters_Filters_Simple_Regex_10();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::0"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::1"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::2"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::3"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::0"))
        return Test_FilterManager_ParseFilters_Filters_Divers_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::1"))
        return Test_FilterManager_ParseFilters_Filters_Divers_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::2"))
        return Test_FilterManager_ParseFilters_Filters_Divers_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::3"))
        return Test_FilterManager_ParseFilters_Filters_Divers_3();
    
    assert(0);

    return false;
}