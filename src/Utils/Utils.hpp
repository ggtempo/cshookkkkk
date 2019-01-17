#pragma once
#include <memory>
#include <optional>
#include <cstdint>
#include <vector>
#include <tuple>

#include <Windows.h>

namespace Utils
{
	struct ModuleInfo
	{
		HINSTANCE hInstance;
		uint32_t fwdReason;
		uint32_t pid;
	};

    BOOL CALLBACK EnumProcCallback(HWND hWnd, LPARAM lParam)
    {
        static auto is_main_window = [](HWND handle) {
            return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
        };

        auto& data = *(std::pair<uint32_t, std::vector<HWND>>*)lParam;

        unsigned long process_id = 0;
        GetWindowThreadProcessId(hWnd, &process_id);
        if (data.first != process_id || !is_main_window(hWnd))
            return TRUE;

        data.second.push_back(hWnd);
        return FALSE;
    }

	inline HWND GetMainWindow(uint32_t pid)
	{
		std::pair<uint32_t, std::vector<HWND>> data;
		data.first = pid;

		EnumWindows(EnumProcCallback, (LPARAM)&data);

		// Just search for the largest window ...
		HWND largestHandle = 0;
		int32_t largestSize = 0;

		for (auto window : data.second)
		{
			RECT r = {};
			GetWindowRect(window, &r);

			int32_t size = r.right * r.bottom;
			if (size > largestSize)
			{
				largestHandle = window;
				largestSize = size;
			}
		}

		if (!IsWindow(largestHandle))
		{
			auto i = 5;
		}

		return largestHandle;
	}
	
	inline std::optional<std::wstring> GetBasePath(HMODULE hModule)
	{
		wchar_t buf[MAX_PATH];
		if (auto result = GetModuleFileName(hModule, buf, sizeof(wchar_t) * MAX_PATH); result > 0)
		{
			auto baseName = std::wstring(buf);
			auto last = baseName.find_last_of(L"\\");
			return baseName.substr(0, last + 1);
		}
		else
		{
			return std::nullopt;
		}
	}
}