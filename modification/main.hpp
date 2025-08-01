//
// Created by pmacc on 9/21/2024.
//

#pragma once

#include <hadesmem/process.hpp>
#include <hadesmem/patcher.hpp>

#include <Windows.h>

#include <cstdint>
#include <memory>
#include <atomic>

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>

#include "game.hpp"
#include "types.h"

namespace AndrgitWoWMod {
    constexpr uint32_t MAJOR_VERSION = 1;
    constexpr uint32_t MINOR_VERSION = 0;
    constexpr uint32_t PATCH_VERSION = 5;

    /* Configurable settings set by user */
    extern UserSettings gUserSettings;

    using RangeCheckSelectedT = bool(__fastcall*)(uintptr_t* playerUnit, const game::SpellRec*,
        std::uint64_t targetGuid, char ignoreErrors);
    using CGGameUI_TargetT = void(__stdcall*)(uint64_t objectGUID);
    using Spell_C_TargetSpellT = bool(__fastcall*)(
        uint32_t* player,
        uint32_t* spellId,
        uint32_t unk3,
        float unk4);
    using Spell_C_IsSpellUsableT = int(__fastcall*)(const game::SpellRec* spellRec, uint32_t* usesManaReturn);

    using LoadScriptFunctionsT = void(__stdcall*)();
    using FrameScript_RegisterFunctionT = void(__fastcall*)(char* name, uintptr_t* func);
    using FrameScript_CreateEventsT = void(__fastcall*)(int param_1, uint32_t maxEventId);

    using LuaScriptT = uint32_t(__fastcall*)(uintptr_t* luaState);
    using GetGUIDFromNameT = std::uint64_t(__fastcall*)(const char*);
    using lua_isstringT = bool(__fastcall*)(uintptr_t*, int);
    using lua_isnumberT = bool(__fastcall*)(uintptr_t*, int);
    using lua_tostringT = char* (__fastcall*)(uintptr_t*, int);
    using lua_tonumberT = double(__fastcall*)(uintptr_t*, int);
    using lua_gettopT = int(__fastcall*)(uintptr_t*);
    using lua_pushnumberT = void(__fastcall*)(uintptr_t*, double);
    using lua_pushstringT = void(__fastcall*)(uintptr_t*, char*);
    using lua_errorT = void(__cdecl*)(uintptr_t*, const char*);

    using GetSpellSlotAndBookTypeFromSpellNameT = uint32_t(__fastcall*)(const char*, uint32_t*);

    using SpellVisualsInitializeT = void(__stdcall*)(void);

    using CVarLookupT = uintptr_t * (__fastcall*)(const char*);
    using SetCVarT = int(__fastcall*)(uintptr_t* luaPtr);
    using CVarRegisterT = int* (__fastcall*)(char* name, char* help, int unk1, const char* defaultValuePtr,
        void* callbackPtr,
        int category, char unk2, int unk3);

    uint32_t GetTime();

    std::string GetHumanReadableTime();

    void RegisterLuaFunction(char*, uintptr_t* func);

    void updateFromCvar(const char* cvar, const char* value);

    int Script_SetCVarHook(hadesmem::PatchDetourBase* detour, uintptr_t* luaPtr);

    int* getCvar(const char* cvar);

    void loadUserVar(const char* cvar);

    void loadConfig();

    void initHooks();

    void SpellVisualsInitializeHook(hadesmem::PatchDetourBase* detour);

    void LoadScriptFunctionsHook(hadesmem::PatchDetourBase* detour);
}