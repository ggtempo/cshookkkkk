#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <tuple>
#include <Windows.h>
#include <Psapi.h>

namespace Memory
{
	inline uint32_t SetMemoryProtection(uintptr_t base, size_t size, uint32_t protection)
	{
		uint32_t oldProtection;
		VirtualProtect((LPVOID)base, size, protection, (PDWORD)&oldProtection);
		return oldProtection;
	}

	inline uintptr_t CreateExecutableMemory(const std::vector<uint8_t>& originalBytes, uintptr_t callAddress, uintptr_t returnAddress)
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
		SetMemoryProtection(reinterpret_cast<uintptr_t>(result), funcSize, PAGE_EXECUTE_READWRITE);

		return reinterpret_cast<uintptr_t>(result);
	}

	inline uintptr_t HookFunc(uintptr_t hookAt, uintptr_t hookFunc, size_t neededBytes)
	{
		// Set protection so that we can read, write and execute
		auto oldProtection = SetMemoryProtection(hookAt, neededBytes, PAGE_EXECUTE_READWRITE);

		// Collect necessary bytes
		std::vector<uint8_t> bytes;
		for (auto i = 0; i < neededBytes; i++)
		{
			bytes.push_back(*(uint8_t*)(hookAt + i));
		}

		auto newFunc = CreateExecutableMemory(bytes, hookFunc, hookAt + 5);
		intptr_t newOffset = newFunc - hookAt - 5;								// 32bit relative jumps ( - 5 because relative to next instruction )

		*(uint8_t*)(hookAt) = 0xE9;		// Jump - 0xE9 0x00000000 - JMP 32bit relative
		*(uint32_t*)(((uint8_t*)hookAt) + 1) = newOffset;

		SetMemoryProtection(hookAt, neededBytes, oldProtection);

		return newOffset;
	}

	inline std::pair<uintptr_t, uintptr_t> CreateExecutableMemory2(const std::vector<uint8_t>& originalBytes, uintptr_t callAddress, uintptr_t returnAddress)
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
		SetMemoryProtection(reinterpret_cast<uintptr_t>(result), funcSize, PAGE_EXECUTE_READWRITE);

		return std::make_pair(reinterpret_cast<uintptr_t>(result), postHookAddress);
	}

	inline uintptr_t HookFunc2(uintptr_t hookAt, uintptr_t hookFunc, size_t neededBytes)
	{
		// Set protection so that we can read, write and execute
		auto oldProtection = SetMemoryProtection(hookAt, neededBytes, PAGE_EXECUTE_READWRITE);

		// Collect necessary bytes
		std::vector<uint8_t> bytes;
		for (auto i = 0; i < neededBytes; i++)
		{
			bytes.push_back(*(uint8_t*)(hookAt + i));
		}

		auto newFunc = CreateExecutableMemory2(bytes, hookFunc, hookAt + 5);
		intptr_t newOffset = newFunc.first - hookAt - 5;						// 32bit relative jumps ( - 5 because relative to next instruction )

		*(uint8_t*)(hookAt) = 0xE9;		// Jump - 0xE9 0x00000000 - JMP 32bit relative
		*(uint32_t*)(((uint8_t*)hookAt) + 1) = newOffset;

		SetMemoryProtection(hookAt, neededBytes, oldProtection);

		return newFunc.second;
	}

	inline uint32_t FindPattern(uint8_t* base, size_t size, std::vector<uint8_t> bytes, int32_t add = 0, bool substractModule = false)
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

    inline MODULEINFO GetModuleInfo(std::string moduleName)
    {
        MODULEINFO info;
		uint8_t* moduleHandle = (uint8_t*)GetModuleHandleA(moduleName.c_str());
		GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleHandle, &info, sizeof(info));

        return info;
    }

	inline uint32_t FindPattern(std::string moduleName, std::vector<uint8_t> bytes, int32_t add = 0, bool substractModule = false)
	{
		MODULEINFO info;
		uint8_t* moduleHandle = (uint8_t*)GetModuleHandleA(moduleName.c_str());
		GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleHandle, &info, sizeof(info));
		return FindPattern(reinterpret_cast<uint8_t*>(info.lpBaseOfDll), info.SizeOfImage, bytes, add, substractModule);
	}

    // Simple RAII memory protection class
    class MemoryProtect
	{
		private:
			uint32_t base;
			size_t size;
			uint32_t old;

		public:
			MemoryProtect(void* base, size_t size, uint32_t level)
			{
				this->base = (uint32_t)base;
				this->size = size;
				this->old = 0;

				VirtualProtect(base, this->size, level, (PDWORD)&this->old);
			}

			~MemoryProtect()
			{
				VirtualProtect((void*)this->base, this->size, this->old, NULL);
			}
	};


    // VMT Hooking class
    class VMTHook
	{
		public:
			uint32_t** base;
			uint32_t* oldVMT;
			uint32_t* newVMT;
			size_t size;
			bool hooked;

			std::unique_ptr<MemoryProtect> protect;

		public:
			VMTHook(void* base)
			{
				this->base = (uint32_t**)base;
				this->oldVMT = *(uint32_t**)base;
				this->size = this->Size();
				this->newVMT = new uint32_t[size];
				this->hooked = false;

				memcpy(this->newVMT, this->oldVMT, this->size * sizeof(uint32_t));
			}

			~VMTHook()
			{
				this->UnHook();
				delete[] this->newVMT;
			}

			size_t Size()
			{
				size_t i = 0;
				while (this->oldVMT[i])
					i++;

				return i;
			}

			void Hook()
			{
				if (!this->hooked)
				{
					this->protect = std::make_unique<MemoryProtect>(this->base, this->size, PAGE_READWRITE);
					*(uint32_t**)(this->base) = this->newVMT;
					this->hooked = true;
				}
			}

			void UnHook()
			{
				*(uint32_t**)(this->base) = this->oldVMT;
				this->hooked = false;
				this->protect.release();
			}

			void HookVFunc(void* func, uint32_t position)
			{
				if (position > this->size)
					return;

				this->newVMT[position] = (uint32_t)func;
			}

			template <typename T>
			T GetVFunc(uint32_t position)
			{
				if (position > this->size)
					return (T)nullptr;

				return (T)this->newVMT[position];
			}

			template <typename T>
			T GetOVFunc(uint32_t position)
			{
				if (position > this->size)
					return (T)nullptr;

				return (T)this->oldVMT[position];
			}

            template <typename T>
            T GetBase()
            {
                return reinterpret_cast<T>(this->base);
            }
	};

	template <typename T>
	T GetVFunction(void* base, uint32_t position)
	{
		uint32_t* VMT = *(uint32_t**)base;

		return (T)(VMT[position]);
	}

}