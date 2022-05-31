#include "WmiQuery.hpp"

/*
    Credits:
    https://stackoverflow.com/a/48741695
*/

Wmi::WmiQueryResult Wmi::GetWmiQueryResult(std::wstring WmiQuery, std::wstring PropNameOfResultObject, bool AllowEmptyItems)
{
    WmiQueryResult ReturnValue;
    ReturnValue.Error = WmiQueryError::None;
    ReturnValue.ErrorDescription = L"";

    HRESULT Result;

    IWbemLocator* Locator = NULL;
    IWbemServices* Services = NULL;
    IEnumWbemClassObject* Enumerator = NULL;
    IWbemClassObject* ClassObject = NULL;
    VARIANT VariantProp;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    Result = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(Result))
    {
        ReturnValue.Error = WmiQueryError::ComInitializationFailure;
        ReturnValue.ErrorDescription = L"Failed to initialize COM library. Error code : " + std::to_wstring(Result);
    }
    else
    {
        // Step 2: --------------------------------------------------
        // Set general COM security levels --------------------------
        // note: JUCE Framework users should comment this call out,
        // as this does not need to be initialized to run the query.
        // see https://social.msdn.microsoft.com/Forums/en-US/48b5626a-0f0f-4321-aecd-17871c7fa283/unable-to-call-coinitializesecurity?forum=windowscompatibility 
        Result = CoInitializeSecurity(
            NULL,
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            NULL                         // Reserved
        );


        if (FAILED(Result))
        {
            ReturnValue.Error = WmiQueryError::SecurityInitializationFailure;
            ReturnValue.ErrorDescription = L"Failed to initialize security. Error code : " + std::to_wstring(Result);
        }
        else
        {
            // Step 3: ---------------------------------------------------
            // Obtain the initial locator to WMI -------------------------
            Locator = NULL;

            Result = CoCreateInstance(
                CLSID_WbemLocator,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, (LPVOID*)&Locator);

            if (FAILED(Result))
            {
                ReturnValue.Error = WmiQueryError::IWbemLocatorFailure;
                ReturnValue.ErrorDescription = L"Failed to create IWbemLocator object. Error code : " + std::to_wstring(Result);
            }
            else
            {
                // Step 4: -----------------------------------------------------
                // Connect to WMI through the IWbemLocator::ConnectServer method

                Services = NULL;

                // Connect to the root\cimv2 namespace with
                // the current user and obtain pointer pSvc
                // to make IWbemServices calls.
                Result = Locator->ConnectServer(
                    _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
                    NULL,                    // User name. NULL = current user
                    NULL,                    // User password. NULL = current
                    0,                       // Locale. NULL indicates current
                    NULL,                    // Security flags.
                    0,                       // Authority (for example, Kerberos)
                    0,                       // Context object 
                    &Services                    // pointer to IWbemServices proxy
                );

                // Connected to ROOT\\CIMV2 WMI namespace

                if (FAILED(Result))
                {
                    ReturnValue.Error = WmiQueryError::IWbemServiceConnectionFailure;
                    ReturnValue.ErrorDescription = L"Could not connect to Wbem service.. Error code : " + std::to_wstring(Result);
                }
                else
                {
                    // Step 5: --------------------------------------------------
                    // Set security levels on the proxy -------------------------

                    Result = CoSetProxyBlanket(
                        Services,                        // Indicates the proxy to set
                        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
                        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
                        NULL,                        // Server principal name 
                        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
                        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                        NULL,                        // client identity
                        EOAC_NONE                    // proxy capabilities 
                    );

                    if (FAILED(Result))
                    {
                        ReturnValue.Error = WmiQueryError::BlanketProxySetFailure;
                        ReturnValue.ErrorDescription = L"Could not set proxy blanket. Error code : " + std::to_wstring(Result);
                    }
                    else
                    {
                        // Step 6: --------------------------------------------------
                        // Use the IWbemServices pointer to make requests of WMI ----

                        // For example, get the name of the operating system
                        Enumerator = NULL;
                        Result = Services->ExecQuery(
                            bstr_t("WQL"),
                            bstr_t(WmiQuery.c_str()),
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL,
                            &Enumerator);

                        if (FAILED(Result))
                        {
                            ReturnValue.Error = WmiQueryError::BadQueryFailure;
                            ReturnValue.ErrorDescription = L"Bad query. Error code : " + std::to_wstring(Result);
                        }
                        else
                        {
                            // Step 7: -------------------------------------------------
                            // Get the data from the query in step 6 -------------------

                            ClassObject = NULL;
                            ULONG uReturn = 0;

                            while (Enumerator)
                            {
                                HRESULT hr = Enumerator->Next(WBEM_INFINITE, 1,
                                    &ClassObject, &uReturn);

                                if (0 == uReturn)
                                {
                                    break;
                                }

                                // VARIANT vtProp;

                                // Get the value of desired property
                                hr = ClassObject->Get(PropNameOfResultObject.c_str(), 0, &VariantProp, 0, 0);
                                if (S_OK != hr)
                                {
                                    ReturnValue.Error = WmiQueryError::PropertyExtractionFailure;
                                    ReturnValue.ErrorDescription = L"Couldn't extract property: " + PropNameOfResultObject + L" from result of query. Error code : " + std::to_wstring(hr);
                                }
                                else {
                                    BSTR val = VariantProp.bstrVal;

                                    // Sometimes val might be NULL even when result is S_OK
                                    // Convert NULL to empty string (otherwise "std::wstring(val)" would throw exception)
                                    if (NULL == val)
                                    {
                                        if (AllowEmptyItems)
                                        {
                                            ReturnValue.ResultList.push_back(std::wstring(L""));
                                        }
                                    }
                                    else {
                                        ReturnValue.ResultList.push_back(std::wstring(val));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Cleanup
    // ========

    VariantClear(&VariantProp);

    if (ClassObject)
        ClassObject->Release();

    if (Services)
        Services->Release();

    if (Locator)
        Locator->Release();

    if (Enumerator)
        Enumerator->Release();

    CoUninitialize();

    return ReturnValue;
}

std::string Wmi::Query(std::wstring Query, std::wstring PropName)
{
    const WmiQueryResult QueryResult = GetWmiQueryResult(Query, PropName);
    std::string Result;

    if (QueryResult.Error != WmiQueryError::None)
    {
        return {};
    }

    for (const auto& ItemWStr : QueryResult.ResultList)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> WStrConverter;
        const std::string ItemStr = WStrConverter.to_bytes(ItemWStr);

        // Append item to our string
        Result.append(ItemStr);
    }

    return Result;
}
