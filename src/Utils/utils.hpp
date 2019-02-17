#pragma once
#include <memory>
#include <optional>
#include <cstdint>
#include <vector>
#include <tuple>

#include <Windows.h>

namespace utils
{
    struct module_info
    {
        HINSTANCE instance;
        uint32_t reason;
        uint32_t pid;
    };

    inline BOOL CALLBACK enum_proc_callback(HWND hwnd, LPARAM lparam)
    {
        static auto is_main_window = [](HWND handle) {
            return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
        };

        auto& data = *(std::pair<uint32_t, std::vector<HWND>>*)lparam;

        unsigned long process_id = 0;
        GetWindowThreadProcessId(hwnd, &process_id);
        if (data.first != process_id || !is_main_window(hwnd))
            return TRUE;

        data.second.push_back(hwnd);
        return FALSE;
    }

    inline HWND get_main_window(uint32_t pid)
    {
        std::pair<uint32_t, std::vector<HWND>> data;
        data.first = pid;

        EnumWindows(enum_proc_callback, (LPARAM)&data);

        // Just search for the largest window ...
        HWND largest_handle = 0;
        int32_t largest_size = 0;

        for (auto window : data.second)
        {
            RECT r = {};
            GetWindowRect(window, &r);

            int32_t size = r.right * r.bottom;
            if (size > largest_size)
            {
                largest_handle = window;
                largest_size = size;
            }
        }

        if (!IsWindow(largest_handle))
        {
            auto i = 5;
        }

        return largest_handle;
    }
    
    inline std::optional<std::wstring> get_base_path(HMODULE hModule)
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

    inline std::string remove_extension(std::string file_name)
    {
        auto position = file_name.find_last_of('.');
        if (position != std::string::npos)
        {
            return file_name.substr(0, position);
        }

        return file_name;
    }

    inline std::string get_extension(const std::string& file_name)
    {
        auto position = file_name.find_last_of('.');
        if (position != std::string::npos)
        {
            return file_name.substr(position);
        }

        return "";
    }

    inline std::vector<std::string> get_files_in_directory(const std::string& directory)
    {
        // Would love to use std::filesystem
        // Unfortunately, mingw-w64 is broken and std::filesystem causes compile errors
        std::vector<std::string> files;
        auto matcher = directory + "*";
        WIN32_FIND_DATAA data = {};

        auto find = FindFirstFileA(matcher.c_str(), &data);

        do
        {
            if (find != INVALID_HANDLE_VALUE)
            {
                std::string file_name(data.cFileName);

                if (file_name.length() > 2)
                    files.push_back(file_name);
            }
        } while(FindNextFileA(find, &data));

        return files;
    }
}