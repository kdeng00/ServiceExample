#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "Models.h"
#include "ServiceEntryPoint.hpp"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")


template<typename Handle = SC_HANDLE, typename Char = wchar_t>
VOID ServiceInstall(void);

template<typename TStr = LPTSTR, typename CTStr = LPCTSTR,
    typename Handle = HANDLE, typename Char = TCHAR>
VOID ServiceReportEvent(LPTSTR);

std::shared_ptr<models::ServiceInfo<>> 
service_info(new models::ServiceInfo<>(L"Service Example"));


#undef main

int _tmain(int argc, TCHAR *argv[])
{
    std::cout << "ServiceExample\n";

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        ServiceInstall();

        return -1;
    }


    SERVICE_TABLE_ENTRY SERVICETABLE[] =
    {
        { service_info->service_name, 
            (LPSERVICE_MAIN_FUNCTION)
            service::ServiceEntryPoint::service_main_start },
        { nullptr, nullptr },
    };

    if (!StartServiceCtrlDispatcher(SERVICETABLE))
    {
        auto func = reinterpret_cast<LPTSTR>(TEXT("StartServiceCtrlDispatcher"));
        ServiceReportEvent(func);
    }


    return 0;
}



template<typename Handle, typename Char>
VOID ServiceInstall()
{
    Char buffer[MAX_PATH];

    if (!GetModuleFileName(nullptr, buffer, MAX_PATH))
    {
        std::cout << "Cannot install service " << GetLastError() << "\n";

        return;
    }

    // Get a handle to the SCM database. 

    Handle schSCManager = OpenSCManager(
        nullptr,                    // local computer
        nullptr,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (nullptr == schSCManager)
    {
        std::cout << "OpenSCManager failed " << GetLastError() << "\n";

        return;
    }

    // Create the service

    Handle schService = CreateService(
        schSCManager,              // SCM database 
        service_info->service_name,                   // name of service 
        service_info->service_name,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        buffer,                    // path to service's binary 
        nullptr,                      // no load ordering group 
        nullptr,                      // no tag identifier 
        nullptr,                      // no dependencies 
        nullptr,                      // LocalSystem account 
        nullptr);                     // no password 

    if (schService == nullptr)
    {
        std::cout << "CreateService failed " << GetLastError() << "\n";

        CloseServiceHandle(schSCManager);
        return;
    }
    else
    {
        std::cout << "Service installed successfully\n";
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

template<typename TStr, typename CTStr,
    typename Handle, typename Char>
VOID ServiceReportEvent(LPTSTR szFunction)
{
    CTStr lpszStrings[2];
    Char Buffer[80];

    Handle hEventSource = RegisterEventSource(nullptr, service_info->service_name);

    if (nullptr != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = service_info->service_name;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            SERVICE_ERROR_NORMAL, // event identifier
            nullptr,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            nullptr);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

