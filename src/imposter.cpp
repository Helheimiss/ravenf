#include <string>
#include <vector>
#include <limits>
#include <windows.h>

#include "imposter.hpp"
#include "Memor.hpp"

constexpr uintptr_t         BASE_ADDRESS {0x01A6D6A0};
constexpr std::string_view  MODULE {"UnityPlayer.dll"};


void MakeGoodWeapon() {
    int *bullets = Memor::Intern::RWChainT<int>(MODULE, BASE_ADDRESS, {0x120, 0x38, 0x60, 0x110, 0xB0, 0x130});
    *bullets = std::numeric_limits<int>::max();


    std::vector<uintptr_t> CurrentWeaponConfig {0x120, 0x38, 0x60, 0x110, 0xB0, 0x40, 0x70};
    int *automatic = Memor::Intern::RWChainT<int>(MODULE, BASE_ADDRESS, CurrentWeaponConfig);
    *automatic = 1;


    CurrentWeaponConfig.back() = 0x78;
    for (int i = 0; i < 8; ++i) {
        int *temp = Memor::Intern::RWChainT<int>(MODULE, BASE_ADDRESS, CurrentWeaponConfig);
        *temp = 0;
        CurrentWeaponConfig.back() += sizeof(int);
    }


    CurrentWeaponConfig.back() = 0xB4;
    for (int i = 0; i < 15; ++i) {
        int *temp = Memor::Intern::RWChainT<int>(MODULE, BASE_ADDRESS, CurrentWeaponConfig);
        *temp = 0;
        CurrentWeaponConfig.back() += sizeof(int);
    }
}


void MakeGoodActor() {
    std::vector<uintptr_t> Actor {0x120, 0x38, 0x60, 0x110, 0x128};


    for (int i = 0; i < 4; ++i) {
        float *temp = Memor::Intern::RWChainT<float>(MODULE, BASE_ADDRESS, Actor);
        *temp = std::numeric_limits<float>::max();
        Actor.back() += sizeof(int);
    }


    Actor.back() = 0x360;
    float *speedMultiplier = Memor::Intern::RWChainT<float>(MODULE, BASE_ADDRESS, Actor);
    *speedMultiplier = 5.0f;
}


#ifdef DEBUG
void MsgBoxDBG(const char *text) {
    MessageBoxA(0, text, "Debug", MB_OK);
}
#else
void MsgBoxDBG(const char *text) {
}
#endif


void cheat() {
    MakeGoodWeapon();
    MakeGoodActor();
}
