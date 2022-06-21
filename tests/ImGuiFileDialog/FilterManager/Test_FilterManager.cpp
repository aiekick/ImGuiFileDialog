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

// must be good {filter0, filter1}
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_CollectionFilters_0()
{
    IGFD::FilterManager mgr;
    mgr.ParseFilters("Source Files{.cpp,.c}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// missing end }
// 2022/06/21 => give an infinite loop
bool Test_FilterManager_ParseFilters_CollectionFilters_1()
{
    IGFD::FilterManager mgr;
    mgr.ParseFilters("Source Files{.cpp,.c");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// missing start }
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_CollectionFilters_2()
{
    IGFD::FilterManager mgr;
    mgr.ParseFilters("Source Files.cpp,.c}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v) vTest == std::string(v)
bool Test_FilterManager(const std::string& vTest)
{
	if (IfTestExist("IGFD::FilterManager::ParseFilters::CollectionFilters::0")) return Test_FilterManager_ParseFilters_CollectionFilters_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::CollectionFilters::1")) return Test_FilterManager_ParseFilters_CollectionFilters_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::CollectionFilters::2")) return Test_FilterManager_ParseFilters_CollectionFilters_2();

    assert(0);

    return false;
}