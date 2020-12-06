#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")

#include "Models.h"
#include "ServiceEntryPoint.hpp"
#include "ServiceControlHandler.hpp"


#define SVCNAME TEXT("ServiceExample")
// #define SERVICE_NAME _T("My Service Example")

// service::ServiceControlHandler::g_ServiceStatus = {0};
// SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
// service::ServiceControlHandler::g_StatusHandle = nullptr;
SERVICE_STATUS g_ServiceStatus;
		// static SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
SERVICE_STATUS_HANDLE g_StatusHandle;

HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

// template<typename D = DWORD, typename LStr = LPTSTR>
// VOID WINAPI ServiceMain(D, LStr *);
VOID ServiceInstall(void);
VOID WINAPI ServiceMain(DWORD, LPTSTR*);
VOID WINAPI ServiceCtrlHandler(DWORD);
// DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

VOID ReportServiceStatus(DWORD, DWORD, DWORD);
VOID ServiceInit(DWORD, LPTSTR*);
VOID ServiceReportEvent(LPTSTR);

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

// models::ServiceInfo<> service_info(L"Service Example");
std::shared_ptr<models::ServiceInfo<>> service_info(new models::ServiceInfo<>(L"Service Example"));


#undef main

// int main()
int _tmain(int argc, TCHAR *argv[])
// int __cdel_tmain(int argc, TCHAR *argv[])
{
	std::cout << "ServiceExample\n";

	auto v = TEXT("sss");

	// service::ServiceControlHandler::g_ServiceStatus = {0};
	// SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
	// service::ServiceControlHandler::g_StatusHandle = nullptr;

	/**
	SERVICE_TABLE_ENTRY SERVICETABLE[] =
	{
		// { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain<DWORD, LPTSTR> },
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
		{nullptr, nullptr}
	};
	*/

	// if (StartServiceCtr)

	if (lstrcmpi(argv[1], TEXT("install")) == 0)
	{
		ServiceInstall();

		return -1;
	}

	service::ServiceEntryPoint entry;
	// models::ServiceInfo<> service_info(TEXT("ServiceExample"));
	// models::ServiceInfo<> service_info(L"Service Example");

	SERVICE_TABLE_ENTRY SERVICETABLE[] =
	{
		// { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain<DWORD, LPTSTR> },
		// { SVCNAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		// { service_info.service_name, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		// { (const_cast<wchar_t*>(service_info.service_name)) , (LPSERVICE_MAIN_FUNCTION)entry.service_main },
		// { SVCNAME, (LPSERVICE_MAIN_FUNCTION)entry.service_main_start},
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION)service::ServiceEntryPoint::service_main_start },
		{ nullptr, nullptr },
	};

	if (!StartServiceCtrlDispatcher(SERVICETABLE))
	{
		auto func = reinterpret_cast<LPTSTR>(TEXT("StartServiceCtrlDispatcher"));
		ServiceReportEvent(func);
	}


	return 0;
}



VOID ServiceInstall()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	// TCHAR szPath[MAX_PATH];
	wchar_t buffer[MAX_PATH];

	// if (!GetModuleFileName("", szPath, MAX_PATH))
	if (!GetModuleFileName(nullptr, buffer, MAX_PATH))
	{
		std::cout << "Cannot install service " << GetLastError() << "\n";

		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		std::cout << "OpenSCManager failed " << GetLastError() << "\n";

		return;
	}

	// Create the service

	schService = CreateService(
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		// szPath,                    // path to service's binary 
		buffer,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		// printf("CreateService failed (%d)\n", GetLastError());
		std::cout << "CreateService failed " << GetLastError() << "\n";

		CloseServiceHandle(schSCManager);
		return;
	}
	else
	{
		// printf("Service installed successfully\n");
		std::cout << "Service installed successfully\n";
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

VOID ServiceReportEvent(LPTSTR szFunction)
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	TCHAR Buffer[80];

	hEventSource = RegisterEventSource(NULL, SVCNAME);

	if (NULL != hEventSource)
	{
		StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

		lpszStrings[0] = SVCNAME;
		lpszStrings[1] = Buffer;

		ReportEvent(hEventSource,        // event log handle
			EVENTLOG_ERROR_TYPE, // event type
			0,                   // event category
			// SVC_ERROR,           
			SERVICE_ERROR_NORMAL, // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}


VOID ReportServiceStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	g_ServiceStatus.dwCurrentState = dwCurrentState;
	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	g_ServiceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
	}
	else
	{
		g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
	{
		g_ServiceStatus.dwCheckPoint = 0;
	}
	else
	{
		g_ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		SetEvent(g_ServiceStopEvent);
		ReportServiceStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);

		return;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	}
}





VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
// template<typename D, typename LStr>
// VOID WINAPI ServiceMain(D argc, LStr *argv)
{
	g_StatusHandle = RegisterServiceCtrlHandler(
		SVCNAME,
		ServiceCtrlHandler);

	if (!g_StatusHandle)
	{
		// Report the service event
		return;
	}



	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	* Perform tasks necessary to start the service here
	*/

	// Create a service stop event to wait on later
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		return;
	}


	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);


	/*
	* Perform any cleanup tasks
	*/

	CloseHandle(g_ServiceStopEvent);

	// Tell the service controller we are stopped
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	// const auto some_file_path = "C:\\Users\\brahmix\\example_file.txt";
	const auto some_file_path = "C:\\example_file.txt";

	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		std::fstream file(some_file_path, std::ios::out | std::ios::app);

		const auto content = "test";

		file << content << "\n";

		file.close();

		Sleep(3000);
	}


	return ERROR_SUCCESS;
}

