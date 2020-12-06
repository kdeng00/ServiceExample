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
		ServiceEntryPoint() = default;

		template<typename D = DWORD, typename TStr = LPTSTR,
				 typename Handle = HANDLE,
				 typename Obj = models::ServiceInfo<>,
				 typename Ptr = std::shared_ptr<Obj>>
		static VOID WINAPI service_main_start(D argc, TStr *argv)
		{
			extern Ptr service_info;

			service_info->service_handle = RegisterServiceCtrlHandler(
				service_info->service_name,
				service::ServiceControlHandler::ServiceCtrlHandler);

			if (!service_info->service_handle)
			{
				return;
			}

			notify_service_controller_starting(service_info);

			
			if (!starting_service(service_info))
			{
				return;
			}

			notify_service_controller_started(service_info);

			// Start a thread that will perform the main task of the service
			Handle hThread = CreateThread(nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);

			// Wait until our worker thread exits signaling that the service needs to stop
			WaitForSingleObject(hThread, INFINITE);


			close_handle(service_info);
		}
	private:
		template<typename D = DWORD, typename Lp = LPVOID,
				 typename Obj = models::ServiceInfo<>,
				 typename Ptr = std::shared_ptr<Obj>>
		static D WINAPI ServiceWorkerThread(Lp lpParam)
		{
			extern Ptr service_info;

			const auto INTERVAL = 1500;

			const auto some_file_path = "C:\\example_file.txt";

			while (WaitForSingleObject(service_info->service_stop_event, 0) != WAIT_OBJECT_0)
			{
				std::fstream file(some_file_path, std::ios::out | std::ios::app);

				const auto content = "burrow";

				file << content << "\n";

				file.close();

				Sleep(INTERVAL);
			}


			return ERROR_SUCCESS;
		}


		template<typename Obj = models::ServiceInfo<>,
				 typename Ptr = std::shared_ptr<Obj>>
		static bool starting_service(Ptr service_info)
		{
			/*
			* Perform tasks necessary to start the service here
			*/

			// Create a service stop event to wait on later
			service_info->service_stop_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			if (service_info->service_stop_event == nullptr)
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

				return false;
			}

			return true;
		}


		template<typename Obj = models::ServiceInfo<>,
				 typename Ptr = std::shared_ptr<Obj>>
		static void close_handle(Ptr service_info)
		{
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

		template<typename Obj = models::ServiceInfo<>,
				 typename Ptr = std::shared_ptr<Obj>>
		static void notify_service_controller_starting(Ptr service_info)
		{
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
		}

		template<typename Obj = models::ServiceInfo<>,
			     typename Ptr = std::shared_ptr<Obj>>
		static void notify_service_controller_started(Ptr service_info)
		{
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
		}
	}; 
}

#endif