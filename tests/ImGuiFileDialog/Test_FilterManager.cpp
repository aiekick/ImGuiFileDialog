#include "Test_FilterManager.h"

#include <cassert>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

// ".*,.cpp,.h,.hpp" => simple filters
// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters
// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex
// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex

#pragma region Helper

namespace IGFD {
class TestFilterManager {
private:
    FilterManager m_mgr;

public:
    TestFilterManager() = default;
    TestFilterManager(const char* vFilters) {
        m_mgr.ParseFilters(vFilters);
    }
    std::vector<IGFD::FilterInfos>& getFiltersRef() {
        return m_mgr.m_ParsedFilters;
    }
    FilterInfos& getSelectedFilterRef() {
        return m_mgr.m_SelectedFilter;
    }
    IGFD::FilterManager& getMGRRef() {
        return m_mgr;
    }
};

}  // namespace IGFD

#pragma endregion

#pragma region Filters Simple

bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_0() {
    IGFD::TestFilterManager mgr(".*");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != ".*") return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.].*$))") return false;


    return true;
}

bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_1() {
    IGFD::TestFilterManager mgr(".*,.cpp,.h,.hpp");

    if (mgr.getFiltersRef().size() != 4U) return false;
    if (mgr.getFiltersRef()[0].title != ".*") return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.].*$))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".hpp") return false;

    return true;
}

// missing '.'
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_2() {
    IGFD::TestFilterManager mgr("hpp");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// empty filter after ','
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_3() {
    IGFD::TestFilterManager mgr(".hpp,");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != ".hpp") return false;

    return true;
}

// good filter
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_4() {
    IGFD::TestFilterManager mgr("..hpp");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "..hpp") return false;

    return true;
}

// good filter, but the second is empty
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_5() {
    IGFD::TestFilterManager mgr("..hpp,");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "..hpp") return false;

    return true;
}

// good filters
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_6() {
    IGFD::TestFilterManager mgr("..hpp,.hpp");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "..hpp") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".hpp") return false;

    return true;
}

// good filter, but the third not
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_7() {
    IGFD::TestFilterManager mgr("..hpp,.hpp,.");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "..hpp") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".hpp") return false;

    return true;
}

// good filters
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_8() {
    IGFD::TestFilterManager mgr("..hpp,.hpp,. .");

    if (mgr.getFiltersRef().size() != 3U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "..hpp") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".hpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != "..") return false;

    return true;
}

#pragma endregion

#pragma region Filters Collection

// must be good {filter0, filter1}
// 2022/06/21 => ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_0() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;

    return true;
}

// missing end }
// 2022/06/21 => give an infinite loop
bool Test_IGFD_FilterManager_ParseFilters_Filters_1() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// missing start }
// 2022/06/21 => ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_2() {
    IGFD::TestFilterManager mgr("Source Files.cpp,.c}");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_3() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx}");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != "Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_4() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx},");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != "Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_5() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx} , ");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != " Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;

    return true;
}

// first { missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_6() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files.hpp,.h,.hxx} , ");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;

    return true;
}

// last { missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_7() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx , .md");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;

    return true;
}

// last { missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_8() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md");

    if (mgr.getFiltersRef().size() != 3U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != " Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".md") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_9() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md,Other Files{.txt,.doc}");

    if (mgr.getFiltersRef().size() != 4U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != " Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".md") return false;
    if (mgr.getFiltersRef()[3].title != "Other Files") return false;
    if (mgr.getFiltersRef()[3].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[3].filters.exist(".txt")) return false;
    if (!mgr.getFiltersRef()[3].filters.exist(".doc")) return false;

    return true;
}

// first { missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_10() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files.txt,.doc}");

    if (mgr.getFiltersRef().size() != 3U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != " Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".md") return false;

    return true;
}

// last { missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_11() {
    IGFD::TestFilterManager mgr("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files{.txt,.doc");

    if (mgr.getFiltersRef().size() != 3U) return false;
    if (mgr.getFiltersRef()[0].title != "Source Files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 2U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (mgr.getFiltersRef()[1].title != " Header Files") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hpp")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".hxx")) return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".md") return false;

    return true;
}

#pragma endregion

#pragma region Filters Simple Regex

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_0() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3}))");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_1() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3}");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_2() {
    IGFD::TestFilterManager mgr("[.][0-9]{3}))");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_3() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,.h,.hpp");

    if (mgr.getFiltersRef().size() != 4U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".hpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_4() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.getFiltersRef().size() != 5U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[4].getFirstFilter() != ".hpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_5() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.getFiltersRef().size() != 5U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[4].getFirstFilter() != ".hpp") return false;

    return true;
}

// last ) missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_6() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,(([.][0-9]{3},.h,.hpp");

    if (mgr.getFiltersRef().size() != 4U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".hpp") return false;

    return true;
}

// first ( missing
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_7() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,[.][0-9]{3})),.h,.hpp");

    if (mgr.getFiltersRef().size() != 4U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".hpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_8() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})),.cpp,(([.][0-9]{3})) ,.h,.hpp");

    if (mgr.getFiltersRef().size() != 5U) return false;
    if (mgr.getFiltersRef()[0].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[1].getFirstFilter() != ".cpp") return false;
    if (mgr.getFiltersRef()[2].getFirstFilter() != "(([.][0-9]{3}))") return false;
    if (mgr.getFiltersRef()[3].getFirstFilter() != ".h") return false;
    if (mgr.getFiltersRef()[4].getFirstFilter() != ".hpp") return false;

    return true;
}

// must fail
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_9() {
    IGFD::TestFilterManager mgr("(([.][0-9]{3})");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

// must fail
bool Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_10() {
    IGFD::TestFilterManager mgr("([.][0-9]{3}))");

    if (mgr.getFiltersRef().size() != 0U) return false;

    return true;
}

#pragma endregion

#pragma region Filters Collection with Regex

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Regex_0() {
    IGFD::TestFilterManager mgr("frames files{(([.][0-9]{3})),.frames}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "frames files") return false;
    if (!mgr.getFiltersRef()[0].filters.exist("(([.][0-9]{3}))")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".frames")) return false;

    return true;
}

// invalid regex
bool Test_IGFD_FilterManager_ParseFilters_Filters_Regex_1() {
    IGFD::TestFilterManager mgr("frames files{((.001,.NNN)),.frames}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "frames files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 1U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".frames")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Regex_2() {
    IGFD::TestFilterManager mgr("frames files(.frames){(([.][0-9]{3})),.frames}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "frames files(.frames)") return false;
    if (!mgr.getFiltersRef()[0].filters.exist("(([.][0-9]{3}))")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".frames")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Regex_3() {
    IGFD::TestFilterManager mgr("frames files(.cpp,.hpp){(([.][0-9]{3})),.cpp,.hpp}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "frames files(.cpp,.hpp)") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist("(([.][0-9]{3}))")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".hpp")) return false;

    return true;
}

#pragma endregion

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
bool Test_IGFD_FilterManager_ParseFilters_Filters_Divers_0() {
    IGFD::TestFilterManager mgr(
        "\
All files{.*},\
Frames Format 0(.001,.NNN){(([.][0-9]{3}))},\
Frames Format 1(XXX.png){(([0-9]{3}.png))},\
Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},\
Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},\
.md\
");

    if (mgr.getFiltersRef().size() != 6U) return false;
    if (mgr.getFiltersRef()[0].title != "All files") return false;
    if (mgr.getFiltersRef()[0].filters.size() != 1U) return false;
    if (!mgr.getFiltersRef()[0].filters.exist("(([.].*$))")) return false;
    if (mgr.getFiltersRef()[1].title != "Frames Format 0(.001,.NNN)") return false;
    if (mgr.getFiltersRef()[1].filters.size() != 1U) return false;
    if (!mgr.getFiltersRef()[1].filters.exist("(([.][0-9]{3}))")) return false;
    if (mgr.getFiltersRef()[2].title != "Frames Format 1(XXX.png)") return false;
    if (mgr.getFiltersRef()[2].filters.size() != 1U) return false;
    if (!mgr.getFiltersRef()[2].filters.exist("(([0-9]{3}.png))")) return false;
    if (mgr.getFiltersRef()[3].title != "Source files (*.cpp *.h *.hpp)") return false;
    if (mgr.getFiltersRef()[3].filters.size() != 3U) return false;
    if (!mgr.getFiltersRef()[3].filters.exist(".cpp")) return false;
    if (!mgr.getFiltersRef()[3].filters.exist(".h")) return false;
    if (!mgr.getFiltersRef()[3].filters.exist(".hpp")) return false;
    if (mgr.getFiltersRef()[4].title != "Image files (*.png *.gif *.jpg *.jpeg)") return false;
    if (mgr.getFiltersRef()[4].filters.size() != 4U) return false;
    if (!mgr.getFiltersRef()[4].filters.exist(".png")) return false;
    if (!mgr.getFiltersRef()[4].filters.exist(".gif")) return false;
    if (!mgr.getFiltersRef()[4].filters.exist(".jpg")) return false;
    if (!mgr.getFiltersRef()[4].filters.exist(".jpeg")) return false;
    if (mgr.getFiltersRef()[5].title != ".md") return false;
    if (mgr.getFiltersRef()[5].filters.size() != 1U) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Divers_1() {
    IGFD::TestFilterManager mgr("Regex Custom*.h{((Custom.+[.]h))}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "Regex Custom*.h") return false;
    if (!mgr.getFiltersRef()[0].filters.exist("((Custom.+[.]h))")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Divers_2() {
    IGFD::TestFilterManager mgr("C++ File (*.cpp){.cpp}");

    if (mgr.getFiltersRef().size() != 1U) return false;
    if (mgr.getFiltersRef()[0].title != "C++ File (*.cpp)") return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ParseFilters_Filters_Divers_3() {
    IGFD::TestFilterManager mgr("C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}");

    if (mgr.getFiltersRef().size() != 2U) return false;
    if (mgr.getFiltersRef()[0].title != "C/C++ File (*.c *.cpp)") return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".c")) return false;
    if (!mgr.getFiltersRef()[0].filters.exist(".cpp")) return false;
    if (mgr.getFiltersRef()[1].title != " Header File (*.h)") return false;
    if (!mgr.getFiltersRef()[1].filters.exist(".h")) return false;

    return true;
}
#pragma endregion

#pragma region ReplaceExtentionWithCurrentFilterIfNeeded

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_0() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.cpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_1() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.cpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_2() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".code.cpp", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.code.cpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_3() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".code.cpp", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.code.cpp") return false;

    return true;
}

// if regex, the function have no impact
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_4() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter("((.*\\.a\\.b))", true);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

// many filter in the current collection, no change
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_5() {
    IGFD::TestFilterManager mgr("C/C++ File (*.c,*.cpp){.c,.cpp}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

// one filter in the current collection => change
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_6() {
    IGFD::TestFilterManager mgr("C/C++ File (*.cpp){.cpp}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.cpp") return false;

    return true;
}

// .* => no change
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_7() {
    IGFD::TestFilterManager mgr(".*");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

// one filter .* in one collection => no change
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_8() {
    IGFD::TestFilterManager mgr("All files{.*}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_9() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c.r.x", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c.r.cpp") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_10() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp.tv", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c.r.x", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c.cpp.tv") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_11() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter("", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c.r.x", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c.r.x") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_12() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp.tv", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.cpp.tv") return false;

    return true;
}

// must be ok
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_13() {
    IGFD::TestFilterManager mgr;
    mgr.getSelectedFilterRef().addFilter(".cpp.tv", false);
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.cpp.tv") return false;

    return true;
}

// if regex, the function have no impact
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_14() {
    IGFD::TestFilterManager mgr("C/C++ File{((.*\\.a\\.b))}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_15() {
    IGFD::TestFilterManager mgr("Foobar (*.bar){.bar},All files{.*}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.bar") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_16() {
    IGFD::TestFilterManager mgr("Shader files{.a.b,.comp,.vert,.frag}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.a.b") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_17() {
    IGFD::TestFilterManager mgr("Shader files{.z,.comp,.vert,.frag}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.a.z") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_18() {
    IGFD::TestFilterManager mgr("Shader files{.z,.comp,.vert,.frag}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.a.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto..", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto...", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto....", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;
    res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.....", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_19() {
    IGFD::TestFilterManager mgr("Shader files{((.*)),((.a.*)),.z,.frag}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.c") return false;

    return true;
}

bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_20() {
    IGFD::TestFilterManager mgr("Shader files{.z,((.*)),((.a.*)),.frag}");
    auto res = mgr.getMGRRef().ReplaceExtentionWithCurrentFilterIfNeeded("toto.c", IGFD_ResultMode_OverwriteFileExt);
    if (res != "toto.z") return false;

    return true;
}

// IGFD_ResultMode_OverwriteFileExt
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_21() {
    
   {
        // IGFD_ResultMode_OverwriteFileExt
        // filter {.cpp,.h} with file :
        // toto.h => toto.cpp
        // toto.a.h => toto.a.cpp
        // toto.a. => toto.a.cpp
        // toto.a.h.t => toto.a.h.cpp
        // toto. => toto.cpp
        // toto => toto.cpp
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.cpp,.h}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.h", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.a.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.a.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h.t", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.a.h.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.cpp") return false;
    }

    {
        // IGFD_ResultMode_OverwriteFileExt
        // filter {.z,.a.b} with file :
        // toto.a.h => toto.z
        // toto.a.h.t => toto.a.z
        // toto. => toto.z
        // toto => toto.z
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.z,.a.b}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h.t", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.a.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.z") return false;
    }

    {
        // IGFD_ResultMode_OverwriteFileExt
        // filter {.g.z,.a} with file :
        // toto.a.h => toto.g.z
        // toto.a.h.y => toto.a.g.z
        // toto.a. => toto.g.z
        // toto. => toto.g.z
        // toto => toto.g.z
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.g.z,.a}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.g.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h.y", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.a.g.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.g.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.g.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_OverwriteFileExt);
        if (res != "toto.g.z") return false;
    }

    return true;
}

// IGFD_ResultMode_KeepInputFile
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_22() {

    {
        // IGFD_ResultMode_KeepInputFile
        // filter {.cpp,.h} with file :
        // toto.h => toto.h
        // toto. => toto.
        // toto => toto
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.cpp,.h}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.h", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_KeepInputFile);
        if (res != "toto") return false;
    }

    {
        // IGFD_ResultMode_KeepInputFile
        // filter {.z,.a.b} with file :
        // toto.a.h => toto.a.h
        // toto. => toto.
        // toto => toto
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.z,.a.b}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.a.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_KeepInputFile);
        if (res != "toto") return false;
    }

    {
        // IGFD_ResultMode_KeepInputFile
        // filter {.g.z,.a} with file :
        // toto.a.h => toto.a.h
        // toto. => toto.
        // toto => toto
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.g.z,.a}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.h", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_KeepInputFile);
        if (res != "toto.") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_KeepInputFile);
        if (res != "toto") return false;
    }

    return true;
}

// IGFD_ResultMode_AddIfNoFileExt
bool Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_23() {

     {
        // IGFD_ResultMode_AddIfNoFileExt
        // filter {.cpp,.h} with file :
        // toto.h => toto.h
        // toto.a.h => toto.a.h
        // toto.a. => toto.a.cpp
        // toto. => toto.cpp
        // toto => toto.cpp
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.cpp,.h}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.h", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.a.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.a.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.cpp") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.cpp") return false;
    }

    {
        // IGFD_ResultMode_AddIfNoFileExt
        // filter {.z,.a.b} with file :
        // toto.a.h => toto.a.h
        // toto. => toto.z
        // toto => toto.z
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.z,.a.b}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.a.h", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.a.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.z") return false;
    }

    {
        // IGFD_ResultMode_AddIfNoFileExt
        // filter {.g.z,.a} with file :
        // toto.a.h => toto.a.h
        // toto. => toto.g.z
        // toto => toto.g.z
        IGFD::FilterManager mgr;
        mgr.ParseFilters("files{.g.z,.a}");
        auto res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.h", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.h") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto.", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.g.z") return false;
        res = mgr.ReplaceExtentionWithCurrentFilterIfNeeded("toto", IGFD_ResultMode_AddIfNoFileExt);
        if (res != "toto.g.z") return false;
    }

    return true;
}

#pragma endregion

#pragma region FilterInfos transformAsteriskBasedFilterToRegex

// must be ok
bool Test_IGFD_FilterManager_FilterInfos_transformAsteriskBasedFilterToRegex_0() {
    IGFD::TestFilterManager mgr;
    const auto& res = mgr.getSelectedFilterRef().transformAsteriskBasedFilterToRegex(".a.*");
    if (res != "(([.]a[.].*$))") return false;
    return true;
}

#pragma endregion

#pragma region FilterInfos IsCoveredByFilters

// must be ok
bool Test_IGFD_FilterManager_IsCoveredByFilters_0() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters(".*");

    IGFD::FileInfos file;
    file.fileNameExt      = "toto.a.b";
    file.fileExtLevels[0] = ".a.b";

    if (!mgr.IsCoveredByFilters(file, false)) return false;

    return true;
}

bool Test_IGFD_FilterManager_IsCoveredByFilters_1() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters("{.a.b, .b}");

    {
        IGFD::FileInfos file;
        file.fileNameExt      = "toto.a.b";
        file.fileExtLevels[0] = ".a.b";
        if (!mgr.IsCoveredByFilters(file, false)) return false;
    }
    {
        IGFD::FileInfos file;
        file.fileNameExt      = "toto.b";
        file.fileExtLevels[0] = ".b";
        if (!mgr.IsCoveredByFilters(file, false)) return false;
    }
    {
        IGFD::FileInfos file;
        file.fileNameExt      = "toto.c";
        file.fileExtLevels[0] = ".c";
        if (mgr.IsCoveredByFilters(file, false)) return false;
    }

    return true;
}

#pragma endregion

#pragma region Entry Point


#define IfTestExist(v) \
    if (vTest == std::string(#v)) return v()

bool Test_FilterManager(const std::string& vTest) {
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_0);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_1);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_2);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_3);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_4);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_5);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_6);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_7);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_8);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_0);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_1);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_2);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_3);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_4);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_5);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_6);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_7);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_8);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_9);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_10);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_11);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_0);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_1);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_2);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_3);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_4);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_5);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_6);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_7);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_8);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_9);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_10);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_0);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_1);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_2);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_3);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_0);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_1);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_2);
    IfTestExist(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_3);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_0);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_1);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_2);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_3);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_4);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_5);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_6);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_7);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_8);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_9);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_10);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_11);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_12);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_13);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_14);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_15);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_16);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_17);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_18);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_19);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_20);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_21);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_22);
    IfTestExist(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilterIfNeeded_23);
    IfTestExist(Test_IGFD_FilterManager_FilterInfos_transformAsteriskBasedFilterToRegex_0);
    IfTestExist(Test_IGFD_FilterManager_IsCoveredByFilters_0);
    IfTestExist(Test_IGFD_FilterManager_IsCoveredByFilters_1);
    
    assert(0);

    return false;
}

#pragma endregion
