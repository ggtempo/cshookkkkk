#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <tuple>
#include <Windows.h>
#include <Psapi.h>

namespace memory
{
	inline uint32_t set_memory_protection(uintptr_t base, size_t size, uint32_t protection)
	{
		uint32_t oldProtection;
		VirtualProtect((LPVOID)base, size, protection, (PDWORD)&oldProtection);
		return oldProtection;
	}

	inline uintptr_t create_executable_memory(const std::vector<uint8_t>& originalBytes, uintptr_t callAddress, uintptr_t returnAddress)
	{
		size_t funcSize = 16 + originalBytes.size() + 8;
		uint8_t* result = reinterpret_cast<uint8_t*>(std::malloc(funcSize));
		uint32_t offset = 0;

		auto returnAddressOffset = reinterpret_cast<uintptr_t>(result) + funcSize - 8;
		auto callAddressOffset = reinterpret_cast<uintptr_t>(result) + funcSize - 4;

		result[offset++] = 0x9C;		// 0x9C - PUSHFD
		result[offset++] = 0x60;		// 0x60 - PUSHAD

		// 0xE8 0x00000000 - 0xFF 0x15 0x00000000 - CALL 32bit indirect ....
		result[offset++] = 0xFF;
		result[offset++] = 0x15;
		*(uint32_t*)(result + offset) = callAddressOffset; offset += 4;
		
		result[offset++] = 0x61;		// 0x61 - POPAD
		result[offset++] = 0x9D;		// 0x9D - POPFD
		
		// Restore original bytes
		for (auto i = 0; i < originalBytes.size(); i++)
		{
			result[offset++] = originalBytes[i];
		}

		// Jump to original function - 0xFF 0x25 0x00000000 - JMP 32bit absolute
		result[offset++] = 0xFF;
		result[offset++] = 0x25;
		*(uint32_t*)(result + offset) = returnAddressOffset;

		// Damn indirect jumps ...
		*(uint32_t*)(callAddressOffset) = callAddress;
		*(uint32_t*)(returnAddressOffset) = returnAddress;
		
		// Set protection so that we can read, write and execute
		set_memory_protection(reinterpret_cast<uintptr_t>(result), funcSize, PAGE_EXECUTE_READWRITE);

		return reinterpret_cast<uintptr_t>(result);
	}

	inline uintptr_t hook_func(uintptr_t hookAt, uintptr_t hookFunc, size_t neededBytes)
	{
		// Set protection so that we can read, write and execute
		auto oldProtection = set_memory_protection(hookAt, neededBytes, PAGE_EXECUTE_READWRITE);

		// Collect necessary bytes
		std::vector<uint8_t> bytes;
		for (auto i = 0; i < neededBytes; i++)
		{
			bytes.push_back(*(uint8_t*)(hookAt + i));
		}

		auto newFunc = create_executable_memory(bytes, hookFunc, hookAt + 5);
		intptr_t newOffset = newFunc - hookAt - 5;								// 32bit relative jumps ( - 5 because relative to next instruction )

		*(uint8_t*)(hookAt) = 0xE9;		// Jump - 0xE9 0x00000000 - JMP 32bit relative
		*(uint32_t*)(((uint8_t*)hookAt) + 1) = newOffset;

		set_memory_protection(hookAt, neededBytes, oldProtection);

		return newOffset;
	}

	inline std::pair<uintptr_t, uintptr_t> create_executable_memory2(const std::vector<uint8_t>& originalBytes, uintptr_t callAddress, uintptr_t returnAddress)
	{
		size_t funcSize = 20 + originalBytes.size();
		uint8_t* result = reinterpret_cast<uint8_t*>(std::malloc(funcSize));
		uint32_t offset = 0;

		auto returnAddressOffset = reinterpret_cast<uintptr_t>(result) + funcSize - 8;
		auto callAddressOffset = reinterpret_cast<uintptr_t>(result) + funcSize - 4;

		// 0xE8 0x00000000 - 0xFF 0x25 0x00000000 - JMP 32bit indirect ....
		result[offset++] = 0xFF;
		result[offset++] = 0x25;
		*(uint32_t*)(result + offset) = callAddressOffset; offset += 4;

		uintptr_t postHookAddress = reinterpret_cast<uintptr_t>(result) + offset;

		// Restore original bytes
		for (auto i = 0; i < originalBytes.size(); i++)
		{
			result[offset++] = originalBytes[i];
		}		

		// Jump to original function - 0xFF 0x25 0x00000000 - JMP 32bit absolute
		result[offset++] = 0xFF;
		result[offset++] = 0x25;
		*(uint32_t*)(result + offset) = returnAddressOffset;

		// Damn indirect jumps ...
		*(uint32_t*)(callAddressOffset) = callAddress;
		*(uint32_t*)(returnAddressOffset) = returnAddress;

		// Set protection so that we can read, write and execute
		set_memory_protection(reinterpret_cast<uintptr_t>(result), funcSize, PAGE_EXECUTE_READWRITE);

		return std::make_pair(reinterpret_cast<uintptr_t>(result), postHookAddress);
	}

	inline uintptr_t hook_func2(uintptr_t hookAt, uintptr_t hookFunc, size_t neededBytes)
	{
		// Set protection so that we can read, write and execute
		auto oldProtection = set_memory_protection(hookAt, neededBytes, PAGE_EXECUTE_READWRITE);

		// Collect necessary bytes
		std::vector<uint8_t> bytes;
		for (auto i = 0; i < neededBytes; i++)
		{
			bytes.push_back(*(uint8_t*)(hookAt + i));
		}

		auto newFunc = create_executable_memory2(bytes, hookFunc, hookAt + 5);
		intptr_t newOffset = newFunc.first - hookAt - 5;						// 32bit relative jumps ( - 5 because relative to next instruction )

		*(uint8_t*)(hookAt) = 0xE9;		// Jump - 0xE9 0x00000000 - JMP 32bit relative
		*(uint32_t*)(((uint8_t*)hookAt) + 1) = newOffset;

		set_memory_protection(hookAt, neededBytes, oldProtection);

		return newFunc.second;
	}

	inline uint32_t find_pattern(uint8_t* base, size_t size, std::vector<uint8_t> bytes, int32_t add = 0, bool substractModule = false)
	{
		for (size_t i = 0; i < size; i++)
		{
			bool found = true;
			for (size_t o = 0; o < bytes.size(); o++)
			{
				if (base[i + o] != bytes[o] && bytes[o] != 0x00)
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return *(uint32_t*)(base + i + add) - ((substractModule) ? size : 0);
			}
		}

		return 0;
	}

    inline MODULEINFO get_module_info(std::string moduleName)
    {
        MODULEINFO info;
		uint8_t* moduleHandle = (uint8_t*)GetModuleHandleA(moduleName.c_str());
		GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleHandle, &info, sizeof(info));

        return info;
    }

	inline uint32_t find_pattern(std::string moduleName, std::vector<uint8_t> bytes, int32_t add = 0, bool substractModule = false)
	{
		MODULEINFO info;
		uint8_t* moduleHandle = (uint8_t*)GetModuleHandleA(moduleName.c_str());
		GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleHandle, &info, sizeof(info));
		return find_pattern(reinterpret_cast<uint8_t*>(info.lpBaseOfDll), info.SizeOfImage, bytes, add, substractModule);
	}

    // Simple RAII memory protection class
    class memory_protect
	{
		private:
			uint32_t base;
			size_t size;
			uint32_t old;

		public:
			memory_protect(void* base, size_t size, uint32_t level)
			{
				this->base = (uint32_t)base;
				this->size = size;
				this->old = 0;

				VirtualProtect(base, this->size, level, (PDWORD)&this->old);
			}

			~memory_protect()
			{
				VirtualProtect((void*)this->base, this->size, this->old, NULL);
			}
	};


    // VMT Hooking class
    class vmt_hook
	{
		public:
			uint32_t** base;
			uint32_t* old_vmt;
			uint32_t* new_vmt;
			size_t _size;
			bool hooked;

			std::unique_ptr<memory_protect> protect;

		public:
			vmt_hook(void* base)
			{
				this->base = (uint32_t**)base;
				this->old_vmt = *(uint32_t**)base;
				this->_size = this->size();
				this->new_vmt = new uint32_t[this->_size];
				this->hooked = false;

				memcpy(this->new_vmt, this->old_vmt, this->_size * sizeof(uint32_t));
			}

			~vmt_hook()
			{
				this->unhook();
				delete[] this->new_vmt;
			}

			size_t size()
			{
				size_t i = 0;
				while (this->old_vmt[i])
					i++;

				return i;
			}

			void hook()
			{
				if (!this->hooked)
				{
					this->protect = std::make_unique<memory_protect>(this->base, this->_size, PAGE_READWRITE);
					*(uint32_t**)(this->base) = this->new_vmt;
					this->hooked = true;
				}
			}

			void unhook()
			{
				*(uint32_t**)(this->base) = this->old_vmt;
				this->hooked = false;
				this->protect.release();
			}

			void hook_vfunc(void* func, uint32_t position)
			{
				if (position > this->_size)
					return;

				this->new_vmt[position] = (uint32_t)func;
			}

			template <typename T>
			T get_vfunc(uint32_t position)
			{
				if (position > this->_size)
					return (T)nullptr;

				return (T)this->new_vmt[position];
			}

			template <typename T>
			T get_original_vfunc(uint32_t position)
			{
				if (position > this->_size)
					return (T)nullptr;

				return (T)this->old_vmt[position];
			}

            template <typename T>
            T get_base()
            {
                return reinterpret_cast<T>(this->base);
            }
	};

	template <typename T>
	T get_vfunc(void* base, uint32_t position)
	{
		uint32_t* vmt = *(uint32_t**)base;

		return (T)(vmt[position]);
	}

}