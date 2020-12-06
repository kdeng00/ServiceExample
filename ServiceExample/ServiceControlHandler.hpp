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
        template<typename D = DWORD,
                 typename Obj = models::ServiceInfo<>,
                 typename Ptr = std::shared_ptr<Obj>>
        static VOID WINAPI ServiceCtrlHandler(D ctrlCode)
        {
            extern Ptr service_info;

            switch (ctrlCode)
            {
            case SERVICE_CONTROL_STOP:
                ReportServiceStatus<D>(SERVICE_STOP_PENDING, NO_ERROR, 0);

                SetEvent(service_info->service_stop_event);
                ReportServiceStatus<D>(service_info->service_status.dwCurrentState, NO_ERROR, 0);

                return;
            case SERVICE_CONTROL_INTERROGATE:
                break;
            default:
                break;
            }
        }


        template<typename D = DWORD,
                 typename Obj = models::ServiceInfo<>,
                 typename Ptr = std::shared_ptr<Obj>>
        static VOID ReportServiceStatus(D dwCurrentState,
            D dwWin32ExitCode,
            D dwWaitHint)
        {
            extern Ptr service_info;

            static D dwCheckPoint = 1;

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

            SetServiceStatus(service_info->service_handle, &service_info->service_status);
        }

    private:
    };
}

#endif
