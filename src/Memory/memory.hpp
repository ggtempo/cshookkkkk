#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <tuple>
#include <Windows.h>
#include <Psapi.h>
#include <iostream>

namespace memory
{
    // Simple RAII memory protection class
    class memory_protect
    {
        public:
            memory_protect(uintptr_t base, size_t size, uint32_t protection) : base(base), size(size), old_protection(0)
            {
                this->old_protection = memory_protect::set_memory_protection(base, size, protection);
            }

            ~memory_protect()
            {
                memory_protect::set_memory_protection(this->base, this->size, this->old_protection);
            }

        private:
            uintptr_t base;
            size_t size;
            uint32_t old_protection;

        public:
            static uint32_t set_memory_protection(uintptr_t base, size_t size, uint32_t protection)
            {
                uint32_t old_protection;
                VirtualProtect((LPVOID)base, size, protection, (PDWORD)&old_protection);
                return old_protection;
            }
    };

    // Abstract trampoline hook class
    // Extends into: - Jump hook
    //               - Call hook
    class trampoline_hook
    {
        public:
            trampoline_hook(uintptr_t hook_at, uintptr_t hook_fn, size_t needed_bytes) : hooked(false), hook_at(hook_at), hook_fn(hook_fn), protection(nullptr), allocated_bytes(nullptr), original_function(0)
            {
                this->original_bytes.resize(needed_bytes);
            }

            ~trampoline_hook()
            {
                this->unhook();
            }

            void hook()
            {
                this->hooked = true;

                // Set memory protection at original function
                memory_protect protection(this->hook_at, this->original_bytes.size(), PAGE_EXECUTE_READWRITE);

                // Copy original bytes and NOP them out
                auto location = reinterpret_cast<uint8_t*>(this->hook_at);
                for (auto& original_byte : original_bytes)
                {
                    original_byte = *location;
                    *location++ = 0x90;
                }
                
                this->original_function = this->create_trampoline(this->hook_fn, this->hook_at + 5);

                // Get relative address (start of trampoline - start of hooked function - 5 byte jump to trampoline)
                uintptr_t new_offset = reinterpret_cast<uintptr_t>(this->allocated_bytes) - this->hook_at - 5;   

                *(uint8_t*)(this->hook_at) = 0xE9;		                            // Jump - 0xE9 0x00000000 - JMP 32bit relative
                *(uint32_t*)(((uint8_t*)hook_at) + 1) = new_offset;                 // Jump destination (address relative to next instruction [func + 5 bytes]
            }

            void unhook()
            {
                this->hooked = false;

                // Set memory protecton at original function
                memory_protect protection(this->hook_at, this->original_bytes.size(), PAGE_EXECUTE_READWRITE);

                // Delete the trampoline
                if (this->allocated_bytes)
                    delete this->allocated_bytes;

                // Delete it's protection
                if (this->protection)
                    delete this->protection;

                auto location = reinterpret_cast<uint8_t*>(this->hook_at);

                // Restore original bytes
                for (auto original_byte : this->original_bytes)
                    *location++ = original_byte;
            }

            uintptr_t get_original_function()
            {
                return this->original_function;
            }

        protected:
            virtual uintptr_t create_trampoline(uintptr_t call_address, uintptr_t return_address) = 0;

        protected:
            bool hooked;
            uintptr_t hook_at;
            uintptr_t hook_fn;
            uintptr_t original_function;
            std::vector<uint8_t> original_bytes;

            memory_protect* protection;
            uint8_t* allocated_bytes;
    };

    class call_hook : public trampoline_hook
    {
        public:
            call_hook(uintptr_t hook_at, uintptr_t hook_fn, size_t needed_bytes) : trampoline_hook(hook_at, hook_fn, needed_bytes)
            {
                this->hook();
            }

        protected:
            virtual uintptr_t create_trampoline(uintptr_t call_address, uintptr_t return_address)
            {
                auto total_size = original_bytes.size() + 16 + 8;
                this->allocated_bytes = new uint8_t[total_size];

                // Set location as executable
                this->protection = new memory_protect(reinterpret_cast<uintptr_t>(this->allocated_bytes), total_size, PAGE_EXECUTE_READWRITE);

                auto return_address_offset = reinterpret_cast<uintptr_t>(this->allocated_bytes) + total_size - 8;
                auto call_address_offset = reinterpret_cast<uintptr_t>(this->allocated_bytes) + total_size - 4; 
                
                uint32_t offset = 0;
                
                this->allocated_bytes[offset++] = 0x9C;		// 0x9C - PUSHFD
                this->allocated_bytes[offset++] = 0x60;		// 0x60 - PUSHAD

                // 0xE8 0x00000000 - 0xFF 0x15 0x00000000 - CALL 32bit indirect ....
                this->allocated_bytes[offset++] = 0xFF;
                this->allocated_bytes[offset++] = 0x15;
                *(uint32_t*)(this->allocated_bytes + offset) = call_address_offset; offset += 4;
                
                this->allocated_bytes[offset++] = 0x61;		// 0x61 - POPAD
                this->allocated_bytes[offset++] = 0x9D;		// 0x9D - POPFD

                // Restore original bytes
                for (auto original_byte : this->original_bytes)
                    this->allocated_bytes[offset++] = original_byte;

                // Jump to original function - 0xFF 0x25 0x00000000 - JMP 32bit absolute
                this->allocated_bytes[offset++] = 0xFF;
                this->allocated_bytes[offset++] = 0x25;
                *(uint32_t*)(this->allocated_bytes + offset) = return_address_offset;

                // Damn indirect jumps ...
                *(uint32_t*)(call_address_offset) = call_address;
                *(uint32_t*)(return_address_offset) = return_address;

                return 0;
            }
    };

    class jump_hook : public trampoline_hook
    {
        public:
            jump_hook(uintptr_t hook_at, uintptr_t hook_fn, size_t needed_bytes) : trampoline_hook(hook_at, hook_fn, needed_bytes)
            {
                this->hook();
            }

        protected:
            virtual uintptr_t create_trampoline(uintptr_t call_address, uintptr_t return_address)
            {
                auto total_size = original_bytes.size() + 20;
                this->allocated_bytes = new uint8_t[total_size];

                // Set location as executable
                this->protection = new memory_protect(reinterpret_cast<uintptr_t>(this->allocated_bytes), total_size, PAGE_EXECUTE_READWRITE);

                auto return_address_offset = reinterpret_cast<uintptr_t>(this->allocated_bytes) + total_size - 8;
                auto call_address_offset = reinterpret_cast<uintptr_t>(this->allocated_bytes) + total_size - 4; 
                
                uint32_t offset = 0;
                
                // 0xFF 0x25 0x00000000 - JMP 32bit indirect ....
                this->allocated_bytes[offset++] = 0xFF;
                this->allocated_bytes[offset++] = 0x25;
                *(uint32_t*)(this->allocated_bytes + offset) = call_address_offset; offset += 4;

                uintptr_t post_hook_address = reinterpret_cast<uintptr_t>(this->allocated_bytes) + offset;

                // Restore original bytes
                for (auto original_byte : this->original_bytes)
                    this->allocated_bytes[offset++] = original_byte;

                // Jump to original function - 0xFF 0x25 0x00000000 - JMP 32bit absolute
                this->allocated_bytes[offset++] = 0xFF;
                this->allocated_bytes[offset++] = 0x25;
                *(uint32_t*)(this->allocated_bytes + offset) = return_address_offset;

                // Damn indirect jumps ...
                *(uint32_t*)(call_address_offset) = call_address;
                *(uint32_t*)(return_address_offset) = return_address;

                return post_hook_address;
            }
    };

    inline uint32_t* find_location(uint8_t* base, size_t size, std::vector<uint8_t> bytes, int32_t add = 0, bool substract_module = false)
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
                return (uint32_t*)(base + i + add) - ((substract_module) ? size : 0);
            }
        }

        return 0;
    }

    inline uint32_t find_pattern(uint8_t* base, size_t size, std::vector<uint8_t> bytes, int32_t add = 0, bool substract_module = false)
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
                return *(uint32_t*)(base + i + add) - ((substract_module) ? size : 0);
            }
        }

        return 0;
    }

    inline MODULEINFO get_module_info(std::string module_name)
    {
        MODULEINFO info;
        uint8_t* module_handle = (uint8_t*)GetModuleHandleA(module_name.c_str());
        GetModuleInformation(GetCurrentProcess(), (HMODULE)module_handle, &info, sizeof(info));

        return info;
    }

    inline uint32_t find_pattern(std::string module_name, std::vector<uint8_t> bytes, int32_t add = 0, bool substract_module = false)
    {
        MODULEINFO info;
        uint8_t* module_handle = (uint8_t*)GetModuleHandleA(module_name.c_str());
        GetModuleInformation(GetCurrentProcess(), (HMODULE)module_handle, &info, sizeof(info));

        return find_pattern(reinterpret_cast<uint8_t*>(info.lpBaseOfDll), info.SizeOfImage, bytes, add, substract_module);
    }

    inline uint32_t* find_location(std::string module_name, std::vector<uint8_t> bytes, int32_t add = 0, bool substract_module = false)
    {
        MODULEINFO info;
        uint8_t* module_handle = (uint8_t*)GetModuleHandleA(module_name.c_str());
        GetModuleInformation(GetCurrentProcess(), (HMODULE)module_handle, &info, sizeof(info));

        return find_location(reinterpret_cast<uint8_t*>(info.lpBaseOfDll), info.SizeOfImage, bytes, add, substract_module);
    }

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
                    this->protect = std::make_unique<memory_protect>(reinterpret_cast<uintptr_t>(this->base), this->_size, PAGE_READWRITE);
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
    inline T get_vfunc(void* base, uint32_t position)
    {
        uint32_t* vmt = *(uint32_t**)base;

        return (T)(vmt[position]);
    }

}