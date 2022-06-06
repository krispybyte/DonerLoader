#pragma once
#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <string>
#include <Windows.h>
#include <codecvt>

#pragma comment(lib, "wbemuuid.lib")

/*
    Credits:
    https://stackoverflow.com/a/48741695
*/

namespace Wmi
{
    enum class WmiQueryError
    {
        None,
        BadQueryFailure,
        PropertyExtractionFailure,
        ComInitializationFailure,
        SecurityInitializationFailure,
        IWbemLocatorFailure,
        IWbemServiceConnectionFailure,
        BlanketProxySetFailure,
    };

    struct WmiQueryResult
    {
        std::vector<std::wstring> ResultList;
        WmiQueryError Error = WmiQueryError::None;
        std::wstring ErrorDescription;
    };

    WmiQueryResult GetWmiQueryResult(std::wstring WmiQuery, std::wstring PropNameOfResultObject, bool AllowEmptyItems = false);
    std::string Query(std::wstring Query, std::wstring PropName);
}