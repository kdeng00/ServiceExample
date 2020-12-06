#ifndef MODELS_H_
#define MODELS_H_

#include <string>


namespace models
{
	template<typename Str = std::wstring,
			 typename Char = wchar_t,
			 typename Status = SERVICE_STATUS,
			 typename StatusHandle = SERVICE_STATUS_HANDLE,
			 typename Handle = HANDLE>
	class ServiceInfo
	{
	public:
		ServiceInfo()
		{
			service_stop_event = INVALID_HANDLE_VALUE;
		}
		ServiceInfo(std::wstring &&name) :
			service_name(const_cast<wchar_t*>(name.c_str()))
		{
			service_stop_event = INVALID_HANDLE_VALUE;
		}

		wchar_t *service_name;
		Status service_status;
		StatusHandle service_handle;
		Handle service_stop_event;
	};
}

#endif