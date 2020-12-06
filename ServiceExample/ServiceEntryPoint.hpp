#ifndef SERVICEENTRYPOINT_H_
#define SERVICEENTRYPOINT_H_

#include <fstream>
#include <string>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "Models.h"
#include "ServiceControlHandler.hpp"

namespace service
{
	class ServiceEntryPoint
	{
	public:
		ServiceEntryPoint()
		{
			initialize();
		}

		ServiceEntryPoint(std::wstring name)
		{
			// service_name = static_cast<std::wstring>(name);
			// service_name = name.c_str();
			initialize();
		}

		static VOID WINAPI service_main_start(DWORD argc, LPTSTR *argv)
		{
			// extern models::ServiceInfo<> service_info;
			extern std::shared_ptr<models::ServiceInfo<>> service_info;


			service_info->service_handle = RegisterServiceCtrlHandler(
				// SVCNAME,
				service_info->service_name,
				// ServiceCtrlHandler);
				service::ServiceControlHandler::ServiceCtrlHandler);

			if (!service_info->service_handle)
			{
				// Report the service event
				return;
			}



			// Tell the service controller we are starting
			ZeroMemory(&service_info->service_status, sizeof(service_info->service_status));
			service_info->service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
			service_info->service_status.dwControlsAccepted = 0;
			service_info->service_status.dwCurrentState = SERVICE_START_PENDING;
			service_info->service_status.dwWin32ExitCode = 0;
			service_info->service_status.dwServiceSpecificExitCode = 0;
			service_info->service_status.dwCheckPoint = 0;

			if (SetServiceStatus(service_info->service_handle, &service_info->service_status) == FALSE)
			{
				OutputDebugString(_T(
					"My Sample Service: ServiceMain: SetServiceStatus returned error"));
			}

			/*
			* Perform tasks necessary to start the service here
			*/

			// Create a service stop event to wait on later
			service_info->service_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (service_info->service_stop_event == NULL)
			{
				// Error creating event
				// Tell service controller we are stopped and exit
				service_info->service_status.dwControlsAccepted = 0;
				service_info->service_status.dwCurrentState = SERVICE_STOPPED;
				service_info->service_status.dwWin32ExitCode = GetLastError();
				service_info->service_status.dwCheckPoint = 1;

				if (SetServiceStatus(service_info->service_handle, &service_info->service_status) == FALSE)
				{
					OutputDebugString(_T(
						"My Sample Service: ServiceMain: SetServiceStatus returned error"));
				}

				return;
			}


			// Tell the service controller we are started
			service_info->service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
			service_info->service_status.dwCurrentState = SERVICE_RUNNING;
			service_info->service_status.dwWin32ExitCode = 0;
			service_info->service_status.dwCheckPoint = 0;

			if (SetServiceStatus(service_info->service_handle, &service_info->service_status) == FALSE)
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

			CloseHandle(service_info->service_stop_event);

			// Tell the service controller we are stopped
			service_info->service_status.dwControlsAccepted = 0;
			service_info->service_status.dwCurrentState = SERVICE_STOPPED;
			service_info->service_status.dwWin32ExitCode = 0;
			service_info->service_status.dwCheckPoint = 3;

			if (SetServiceStatus(service_info->service_handle, &service_info->service_status) == FALSE)
			{
				OutputDebugString(_T(
					"My Sample Service: ServiceMain: SetServiceStatus returned error"));
			}
		}



		static DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
		// static DWORD WINAPI ServiceWorkerThread(LPVOID lpParam, 
										 // std::shared_ptr<models::ServiceInfo<>> service_info)
		{
			extern std::shared_ptr<models::ServiceInfo<>> service_info;

			// const auto some_file_path = "C:\\Users\\brahmix\\example_file.txt";
			const auto some_file_path = "C:\\example_file.txt";

			// while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
			while (WaitForSingleObject(service_info->service_stop_event, 0) != WAIT_OBJECT_0)
			{
				std::fstream file(some_file_path, std::ios::out | std::ios::app);

				const auto content = "woof";

				file << content << "\n";

				file.close();

				Sleep(3000);
			}


			return ERROR_SUCCESS;
		}


		template<typename TStr = LPTSTR, typename H = HANDLE,
			typename CSTR = LPCTSTR,
			typename Chr = TCHAR>
			// VOID ServiceReportEvent(LPTSTR szFunction)
			VOID ServiceReportEvent(TStr szFunction)
		{
			// HANDLE hEventSource;
			H hEventSource;
			// LPCTSTR lpszStrings[2];
			CSTR lpszStrings[2];
			// TCHAR Buffer[80];
			CHR Buffer[80];

			hEventSource = RegisterEventSource(NULL, this->service_info.service_name);
			// hEventSource = RegisterEventSource(NULL, L(""));

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
	private:
		models::ServiceInfo<> service_info;

		void initialize()
		{
			// service_info.service_name = L" Server Example";
		}
		/**
		class ServiceInfo;


		service::ServiceEntryPoint::ServiceInfo service_info = {};

		class ServiceInfo
		{
		public:
			ServiceInfo()
			{
			}

			ServiceInfo(std::wstring &&name) :
				service_name(const_cast<wchar_t*>(name.c_str()))
			{

			}

			wchar_t *service_name;
			wchar_t *service_description;
		};
		*/

	}; 
}

#endif