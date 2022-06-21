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

IGFD::FilterManager Test_ParseFilters(const char* vFilters)
{
	IGFD::FilterManager mgr;

#if 0
	mgr.ParseFilters(vFilters);
#else
	std::vector<IGFD::FilterManager::FilterInfos> parsedFilters;

	// ".*,.cpp,.h,.hpp" => simple filters
	// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters
	// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex
	// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex

	bool currentFilterFound = false;

	std::string filter_string = vFilters;

	IGFD::FilterManager::FilterInfos infos;

	bool start_collection = false;
	bool start_normal_filter = false;
	bool start_point = false;

	for (auto c : filter_string)
	{
		if (c == '{')
		{
			start_collection = true;
		}
		else if(c == '}')
		{
			
		}
		else if (c == '(')
		{

		}
		else if (c == ')')
		{

		}
		else if (c == '.')
		{
			start_point = true;
			infos.filter += c;
		}
		else if (c == ',')
		{
			if (!infos.empty())
			{
				parsedFilters.push_back(infos);
			}
			infos.clear();
			start_point = false;
		}
		else
		{
			if (start_point)
				infos.filter += c;
		}
	}

	if (infos.filter.size() == 1U) // just a '.' 
	{
		infos.clear();
	}

	if (!infos.empty())
	{
		parsedFilters.push_back(infos);
	}

	mgr.prParsedFilters = parsedFilters;
#endif
	return mgr;
}

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Simple Filters ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_FilterManager_ParseFilters_Filters_Simple_0()
{
	IGFD::FilterManager mgr = Test_ParseFilters(".*");

	if (mgr.prParsedFilters.size() != 1U) return false;
	if (mgr.prParsedFilters[0].filter != ".*") return false;

	return true;
}

bool Test_FilterManager_ParseFilters_Filters_Simple_1()
{
	IGFD::FilterManager mgr = Test_ParseFilters(".*,.cpp,.h,.hpp");

	if (mgr.prParsedFilters.size() != 4U) return false;
	if (mgr.prParsedFilters[0].filter != ".*") return false;
	if (mgr.prParsedFilters[1].filter != ".cpp") return false;
	if (mgr.prParsedFilters[2].filter != ".h") return false;
	if (mgr.prParsedFilters[3].filter != ".hpp") return false;

	return true;
}

// missing '.'
bool Test_FilterManager_ParseFilters_Filters_Simple_2()
{
	IGFD::FilterManager mgr = Test_ParseFilters("hpp");

	if (mgr.prParsedFilters.size() != 0U) return false;

	return true;
}

// empty filter after ','
bool Test_FilterManager_ParseFilters_Filters_Simple_3()
{
	IGFD::FilterManager mgr = Test_ParseFilters(".hpp,");

	if (mgr.prParsedFilters.size() != 1U) return false;
	if (mgr.prParsedFilters[0].filter != ".hpp") return false;

	return true;
}

// good filter
bool Test_FilterManager_ParseFilters_Filters_Simple_4()
{
	IGFD::FilterManager mgr = Test_ParseFilters("..hpp");

	if (mgr.prParsedFilters.size() != 1U) return false;
	if (mgr.prParsedFilters[0].filter != "..hpp") return false;

	return true;
}

// good filter, but the seocnd is empty
bool Test_FilterManager_ParseFilters_Filters_Simple_5()
{
	IGFD::FilterManager mgr = Test_ParseFilters("..hpp,");

	if (mgr.prParsedFilters.size() != 1U) return false;
	if (mgr.prParsedFilters[0].filter != "..hpp") return false;

	return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_6()
{
	IGFD::FilterManager mgr = Test_ParseFilters("..hpp,.hpp");

	if (mgr.prParsedFilters.size() != 2U) return false;
	if (mgr.prParsedFilters[0].filter != "..hpp") return false;
	if (mgr.prParsedFilters[1].filter != ".hpp") return false;

	return true;
}

// good filter, but the third not
bool Test_FilterManager_ParseFilters_Filters_Simple_7()
{
	IGFD::FilterManager mgr = Test_ParseFilters("..hpp,.hpp,.");

	if (mgr.prParsedFilters.size() != 2U) return false;
	if (mgr.prParsedFilters[0].filter != "..hpp") return false;
	if (mgr.prParsedFilters[1].filter != ".hpp") return false;

	return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_8()
{
	IGFD::FilterManager mgr = Test_ParseFilters("..hpp,.hpp,. .");

	if (mgr.prParsedFilters.size() != 3U) return false;
	if (mgr.prParsedFilters[0].filter != "..hpp") return false;
	if (mgr.prParsedFilters[1].filter != ".hpp") return false;
	if (mgr.prParsedFilters[2].filter != ". .") return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // CollectionFilters /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// must be good {filter0, filter1}
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_0()
{
	IGFD::FilterManager mgr = Test_ParseFilters("Source Files{.cpp,.c}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// missing end }
// 2022/06/21 => give an infinite loop
bool Test_FilterManager_ParseFilters_Filters_Collection_1()
{
    IGFD::FilterManager mgr = Test_ParseFilters("Source Files{.cpp,.c");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// missing start }
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_2()
{
    IGFD::FilterManager mgr = Test_ParseFilters("Source Files.cpp,.c}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v) vTest == std::string(v)
bool Test_FilterManager(const std::string& vTest)
{
	if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::0")) return Test_FilterManager_ParseFilters_Filters_Simple_0();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::1")) return Test_FilterManager_ParseFilters_Filters_Simple_1();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::2")) return Test_FilterManager_ParseFilters_Filters_Simple_2();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::3")) return Test_FilterManager_ParseFilters_Filters_Simple_3();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::4")) return Test_FilterManager_ParseFilters_Filters_Simple_4();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::5")) return Test_FilterManager_ParseFilters_Filters_Simple_5();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::6")) return Test_FilterManager_ParseFilters_Filters_Simple_6();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::7")) return Test_FilterManager_ParseFilters_Filters_Simple_7();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::8")) return Test_FilterManager_ParseFilters_Filters_Simple_8();
	else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::0")) return Test_FilterManager_ParseFilters_Filters_Collection_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::1")) return Test_FilterManager_ParseFilters_Filters_Collection_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::2")) return Test_FilterManager_ParseFilters_Filters_Collection_2();

    assert(0);

    return false;
}