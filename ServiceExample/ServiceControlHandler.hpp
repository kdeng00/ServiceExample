#ifndef SERVICECONTROLHANDLER_H_
#define SERVICECONTROLHANDLER_H_

#include <memory>

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "Models.h"

namespace service
{
	class ServiceControlHandler
	{
	public:
		template<typename D = DWORD>
		static VOID WINAPI ServiceCtrlHandler(D ctrlCode)
		{
			extern std::shared_ptr<models::ServiceInfo<>> service_info;

			switch (ctrlCode)
			{
			case SERVICE_CONTROL_STOP:
				// ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
				ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

				SetEvent(service_info->service_stop_event);
				// ReportServiceStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);
				ReportServiceStatus(service_info->service_status.dwCurrentState, NO_ERROR, 0);

				return;
			case SERVICE_CONTROL_INTERROGATE:
				break;
			default:
				break;
			}
		}
		// static SERVICE_STATUS g_ServiceStatus = {0};
		// static SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
		// static SERVICE_STATUS g_ServiceStatus;
		// static SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
		// static SERVICE_STATUS_HANDLE g_StatusHandle;



		static VOID ReportServiceStatus(DWORD dwCurrentState,
			DWORD dwWin32ExitCode,
			DWORD dwWaitHint)
		{
			extern std::shared_ptr<models::ServiceInfo<>> service_info;

			static DWORD dwCheckPoint = 1;

			service_info->service_status.dwCurrentState = dwCurrentState;
			service_info->service_status.dwWin32ExitCode = dwWin32ExitCode;
			service_info->service_status.dwWaitHint = dwWaitHint;

			if (dwCurrentState == SERVICE_START_PENDING)
			{
				service_info->service_status.dwControlsAccepted = 0;
			}
			else
			{
				service_info->service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
			}

			if ((dwCurrentState == SERVICE_RUNNING) ||
				(dwCurrentState == SERVICE_STOPPED))
			{
				service_info->service_status.dwCheckPoint = 0;
			}
			else
			{
				service_info->service_status.dwCheckPoint = dwCheckPoint++;
			}

			// SetServiceStatus(g_StatusHandle, &service_info->service_status);
			SetServiceStatus(service_info->service_handle, &service_info->service_status);
		}

	private:
	};
}

#endif