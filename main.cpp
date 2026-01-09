#include <windows.h>
#include <iostream>
#include <string>
#include <imposter.hpp>


DWORD injected_thread(LPVOID hModule) {
// #ifdef DEBUG
//     if ( AllocConsole( ) )
//     {
//         FILE* file{};
//         freopen_s(&file, "CONOUT$", "w", stdout);
//         freopen_s(&file, "CONOUT$", "w", stderr);
//         freopen_s(&file, "CONIN$", "r", stdin);
//     }
// #endif

    while (true) {
        if (GetAsyncKeyState('H') & 0x0001) {
            try {
                cheat();
            } catch (std::runtime_error &er) {
                MsgBoxDBG(er.what());
            }
        }

        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            MsgBoxDBG("bye");
// #ifdef DEBUG
//             FreeConsole();
// #endif
            break;
        }

        Sleep(1);
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(hModule), 0);
}

BOOL WINAPI DllMain(
    HMODULE hModule,
    DWORD fdwReason,
    LPVOID lpvReserved )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr,
            0,
            injected_thread,
            hModule,
            0,
            nullptr
        );
    }
    return TRUE;
}