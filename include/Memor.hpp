#pragma once

#include <stdexcept>
#include <vector>
#include <string>

#include <windows.h>
#include <tlhelp32.h>

namespace Memor {
    uintptr_t GetProcessID(LPCTSTR target) noexcept(false);
    uintptr_t GetModBaseAddr(LPCTSTR target, DWORD pid) noexcept(false);

    PROCESSENTRY32 SnapProcess(LPCSTR target, DWORD dwFlags) noexcept(false);
    MODULEENTRY32 SnapModule(LPCSTR target, DWORD pid, DWORD dwFlags) noexcept(false);

    namespace Extern {
        template <typename T>
        T ReadChainT(std::string_view program, std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t>& offsets) noexcept(false);

        template <typename T>
        T WriteChainT(std::string_view program, std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t>& offsets, const T&& newValue) noexcept(false);
    }

    namespace Intern {
        template <typename T>
        T *RWChainT(std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t> &offsets) noexcept(false);
    }

}

inline uintptr_t Memor::GetProcessID(LPCTSTR target) {
    auto res = Memor::SnapProcess(target, TH32CS_SNAPPROCESS);
    return static_cast<uintptr_t>(res.th32ProcessID);

}

inline uintptr_t Memor::GetModBaseAddr(LPCTSTR target, DWORD pid) {
    auto res = Memor::SnapModule(target, pid, TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32);
    return reinterpret_cast<uintptr_t>(res.modBaseAddr);
}

inline PROCESSENTRY32 Memor::SnapProcess(LPCSTR target, DWORD dwFlags) {
    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(dwFlags, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) throw std::runtime_error("CreateToolhelp32Snapshot in SnapProcess failed");

    while(Process32Next( hProcessSnap, &pe32 )) {
        if (strcmp(target, pe32.szExeFile)==0) {
            CloseHandle(hProcessSnap);
            return pe32;
        }
    }

    CloseHandle(hProcessSnap);
    throw std::runtime_error("SnapProcess failed");
}

inline MODULEENTRY32 Memor::SnapModule(LPCSTR target, DWORD pid, DWORD dwFlags) {
    MODULEENTRY32 me32{};
    me32.dwSize = sizeof( MODULEENTRY32 );

    HANDLE hModuleSnap = CreateToolhelp32Snapshot(dwFlags, pid);
    if(hModuleSnap == INVALID_HANDLE_VALUE) throw std::runtime_error("CreateToolhelp32Snapshot in SnapModule failed");

    while (Module32Next(hModuleSnap, &me32)) {
        if (strcmp(target, me32.szModule)==0) {
            CloseHandle(hModuleSnap);
            return me32;
        }
    }

    CloseHandle(hModuleSnap);
    throw std::runtime_error("SnapModule failed");
}

template<typename T>
inline T Memor::Extern::ReadChainT(std::string_view program, std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t> &offsets) {
    uintptr_t pid {};
    uintptr_t moduleAddres {};
    HANDLE hProcess {INVALID_HANDLE_VALUE};

    pid = GetProcessID(program.data());
    if (!pid) throw std::runtime_error("GetProcessID failed");

    if (!module.empty()) {
        moduleAddres = GetModBaseAddr(module.data(), pid);
        if (!moduleAddres) throw std::runtime_error("GetModBaseAddr failed");
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) throw std::runtime_error("OpenProcess failed");


    T returnValue {};
    uintptr_t realAddres = module.empty() ? reinterpret_cast<uintptr_t>(hProcess) : moduleAddres; // TODO()
    if (offsets.empty()) {
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + baseAddres), &returnValue, sizeof(T), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x11");
    }
    else {
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + baseAddres), &realAddres, sizeof(realAddres), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x21");

        for (size_t i = 0; i < offsets.size() - 1; ++i) {
            if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + offsets[i]), &realAddres, sizeof(realAddres), nullptr))
                throw std::runtime_error("ReadProcessMemory failed 0x22");
        }

        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + offsets.back()), &returnValue, sizeof(T), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x23");
    }


    CloseHandle(hProcess);
    return returnValue;
}

template<typename T>
inline T Memor::Extern::WriteChainT(std::string_view program, std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t> &offsets, const T &&newValue) {
    uintptr_t pid {};
    uintptr_t moduleAddres {};
    HANDLE hProcess {INVALID_HANDLE_VALUE};

    pid = GetProcessID(program.data());
    if (!pid) throw std::runtime_error("GetProcessID failed");

    if (!module.empty()) {
        moduleAddres = GetModBaseAddr(module.data(), pid);
        if (!moduleAddres) throw std::runtime_error("GetModBaseAddr failed");
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) throw std::runtime_error("OpenProcess failed");


    T returnValue {};
    uintptr_t realAddres = module.empty() ? reinterpret_cast<uintptr_t>(hProcess) : moduleAddres;
    if (offsets.empty()) {
        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(realAddres + baseAddres), &newValue, sizeof(T), nullptr))
            throw std::runtime_error("WriteProcessMemory failed 0x11");

        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + baseAddres), &returnValue, sizeof(T), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x12");
    }
    else {
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + baseAddres), &realAddres, sizeof(realAddres), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x23");

        for (size_t i = 0; i < offsets.size() - 1; ++i) {
            if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + offsets[i]), &realAddres, sizeof(realAddres), nullptr))
                throw std::runtime_error("ReadProcessMemory failed 0x24");
        }

        if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(realAddres + offsets.back()), &newValue, sizeof(T), nullptr))
            throw std::runtime_error("WriteProcessMemory failed 0x25");

        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(realAddres + offsets.back()), &returnValue, sizeof(T), nullptr))
            throw std::runtime_error("ReadProcessMemory failed 0x26");
    }


    CloseHandle(hProcess);
    return returnValue;
}


template <typename T>
inline T *Memor::Intern::RWChainT(std::string_view module, uintptr_t baseAddres, const std::vector<uintptr_t> &offsets) noexcept(false) {
    uintptr_t address = 0;

    if (module.empty())
        address = reinterpret_cast<uintptr_t>(GetCurrentProcess()) + baseAddres;
    else
        address = reinterpret_cast<uintptr_t>(GetModuleHandleA(module.data())) + baseAddres;


    if (offsets.empty()) {
        if (IsBadReadPtr(reinterpret_cast<LPCVOID>(address), sizeof(T)))
            throw std::runtime_error("bad read 0x1");

        return reinterpret_cast<T *>(address);
    }


    if (IsBadReadPtr(reinterpret_cast<LPCVOID>(address), sizeof(uintptr_t)))
        throw std::runtime_error("bad read 0x2");

    address = *reinterpret_cast<uintptr_t *>(address);
    for (int i = 0; i < offsets.size() - 1; ++i) {
        if (IsBadReadPtr(reinterpret_cast<LPCVOID>(address), sizeof(uintptr_t)))
            throw std::runtime_error("bad read 0x3");

        address += offsets[i];
        address = *reinterpret_cast<uintptr_t *>(address);
    }

    if (IsBadReadPtr(reinterpret_cast<LPCVOID>(address), sizeof(T)))
        throw std::runtime_error("bad read 0x4");

    address += offsets.back();
    return reinterpret_cast<T *>(address);
}