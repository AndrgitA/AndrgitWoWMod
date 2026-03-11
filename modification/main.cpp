/*
    Copyright (c) 2017-2023, namreeb (legal@namreeb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
*/


#include "logging.hpp"
#include "offsets.hpp"
#include "game.hpp"
#include "main.hpp"
#include "spellcast.hpp"
#include "scripts.hpp"
#include "helper.hpp"
#include "nameplate.hpp"

#include <cstdint>
#include <memory>
#include <atomic>

#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

namespace AndrgitWoWMod {
    UserSettings gUserSettings;

    std::unique_ptr<hadesmem::PatchDetour<SpellVisualsInitializeT >> gSpellVisualsInitDetour;
    std::unique_ptr<hadesmem::PatchDetour<LoadScriptFunctionsT >> gLoadScriptFunctionsDetour;
    std::unique_ptr<hadesmem::PatchDetour<FrameScript_CreateEventsT >> gCreateEventsDetour;

    std::unique_ptr<hadesmem::PatchDetour<SetCVarT>> gSetCVarDetour;
    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gIsSpellInRangeDetour;
    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gIsSpellUsableDetour;
    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gGetSpellIdForNameDetour;
    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gGetSpellNameAndRankForIdDetour;
    std::unique_ptr<hadesmem::PatchDetour<Spell_C_TargetSpellT>> gSpell_C_TargetSpellDetour;

    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gGetAndrgitWoWModVersionDetour;
    std::unique_ptr<hadesmem::PatchDetour<LuaScriptT>> gGetDistanceBetween;
    
    bool IsValidPtr(std::uint32_t ptr) {
        return (ptr >= 0x1000 && ptr < 0xFFF00000);
    }

    uint32_t GetTime() {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count()) - gStartTime;
    }

    std::string GetHumanReadableTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm buf;
        localtime_s(&buf, &in_time_t);

        std::stringstream ss;
        ss << std::put_time(&buf, "%Y-%m-%d %X");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    void RegisterLuaFunction(char* name, uintptr_t* func) {
        DEBUG_LOG("Registering " << name << " to " << func);
        auto const registerFunction = reinterpret_cast<FrameScript_RegisterFunctionT>(Offsets::FrameScript_RegisterFunction);
        registerFunction(name, func);
    }

    void updateFromCvar(const char* cvar, const char* value) {
        if (strcmp(cvar, "AWM_QuickcastTargetingSpells") == 0) {
            gUserSettings.quickcastTargetingSpells = atoi(value) != 0;
            DEBUG_LOG("Set AWM_QuickcastTargetingSpells to " << gUserSettings.quickcastTargetingSpells);
        }
        else if (strcmp(cvar, "AWM_NameplateDistance") == 0) {
            auto distance = std::stof(value);
            SetNameplateDistance(distance);
            DEBUG_LOG("Set AWM_NameplateDistance to " << distance);
        }
    }

    int Script_SetCVarHook(hadesmem::PatchDetourBase* detour, uintptr_t* luaPtr) {
        auto const cvarSetOrig = gSetCVarDetour->GetTrampolineT<SetCVarT>();

        auto const lua_isstring = reinterpret_cast<lua_isstringT>(Offsets::lua_isstring);
        if (lua_isstring(luaPtr, 1)) {
            auto const lua_tostring = reinterpret_cast<lua_tostringT>(Offsets::lua_tostring);
            auto const cVarName = lua_tostring(luaPtr, 1);
            auto const cVarValue = lua_tostring(luaPtr, 2);
            // if cvar starts with "AWM_", then we need to handle it
            if (strncmp(cVarName, "AWM_", 4) == 0) {
                updateFromCvar(cVarName, cVarValue);
            }
        } // original function handles errors

        return cvarSetOrig(luaPtr);
    }

    int* getCvar(const char* cvar) {
        auto const cvarLookup = hadesmem::detail::AliasCast<CVarLookupT>(Offsets::CVarLookup);
        uintptr_t* cvarPtr = cvarLookup(cvar);

        if (cvarPtr) {
            return reinterpret_cast<int*>(cvarPtr +
                10); // get intValue from CVar which is consistent, strValue more complicated
        }
        return nullptr;
    }

    void loadUserVar(const char* cvar) {
        int* value = getCvar(cvar);
        if (value) {
            updateFromCvar(cvar, std::to_string(*value).c_str());
        }
        else {
            DEBUG_LOG("Using default value for " << cvar);
        }
    }

    void loadConfig() {
        // logging start
        gStartTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count());

        // remove/rename previous logs
        remove("AndrgitWoWMod_debug.log.3");
        rename("AndrgitWoWMod_debug.log.2", "AndrgitWoWMod_debug.log.3");
        rename("AndrgitWoWMod_debug.log.1", "AndrgitWoWMod_debug.log.2");
        rename("AndrgitWoWMod_debug.log", "AndrgitWoWMod_debug.log.1");

        // open new log file
        debugLogFile.open("AndrgitWoWMod_debug.log");

        DEBUG_LOG("Loading AndrgitWoWMod v" << MAJOR_VERSION << "." << MINOR_VERSION << "." << PATCH_VERSION);

        gUserSettings.quickcastTargetingSpells = false;

        char defaultTrue[] = "1";
        char defaultFalse[] = "0";

        DEBUG_LOG("Registering/Loading CVars");

        // register cvars
        auto const CVarRegister = hadesmem::detail::AliasCast<CVarRegisterT>(Offsets::RegisterCVar);

        char AWM_QuickcastTargetingSpells[] = "AWM_QuickcastTargetingSpells";
        CVarRegister(AWM_QuickcastTargetingSpells, // name
            nullptr, // help
            0,  // unk1
            gUserSettings.quickcastTargetingSpells ? defaultTrue : defaultFalse, // default value address
            nullptr, // callback
            1, // category
            0,  // unk2
            0); // unk3

        char AWM_NameplateDistance[] = "AWM_NameplateDistance";
        CVarRegister(AWM_NameplateDistance, // name
            nullptr, // help
            0,  // unk1
            std::to_string(GetNameplateDistance()).c_str(), // use the game's DAT value as the default
            nullptr, // callback
            1, // category
            0,  // unk2
            0); // unk3

        // update from cvars
        loadUserVar("AWM_QuickcastTargetingSpells");
        loadUserVar("AWM_NameplateDistance");

        nameplateLoad();
    }

    void initHooks() {
        const hadesmem::Process process(::GetCurrentProcessId());

        gSetCVarDetour = createHook<SetCVarT>(process, Offsets::Script_SetCVar, &Script_SetCVarHook);
        gSpell_C_TargetSpellDetour = createHook<Spell_C_TargetSpellT>(process, Offsets::Spell_C_TargetSpell, &Spell_C_TargetSpellHook);
        gIsSpellInRangeDetour = createHook<LuaScriptT>(process, Offsets::Script_IsSpellInRange, Script_IsSpellInRange);
        gIsSpellUsableDetour = createHook<LuaScriptT>(process, Offsets::Script_IsSpellUsable, Script_IsSpellUsable);
        gGetSpellIdForNameDetour = createHook<LuaScriptT>(process, Offsets::Script_GetSpellIdForName, Script_GetSpellIdForName);
        gGetSpellNameAndRankForIdDetour = createHook<LuaScriptT>(process, Offsets::Script_GetSpellNameAndRankForId, Script_GetSpellNameAndRankForId);
        gGetAndrgitWoWModVersionDetour = createHook<LuaScriptT>(process, Offsets::Script_GetAndrgitWoWModVersion, Script_GetAndrgitWoWModVersion);
        gGetDistanceBetween = createHook<LuaScriptT>(process, Offsets::Script_GetDistanceBetween, Script_GetDistanceBetween);

        nameplateInitHooks(process);
    }

    void SpellVisualsInitializeHook(hadesmem::PatchDetourBase* detour) {
        auto const spellVisualsInitialize = detour->GetTrampolineT<SpellVisualsInitializeT>();
        spellVisualsInitialize();
        loadConfig();
        initHooks();
    }

    void LoadScriptFunctionsHook(hadesmem::PatchDetourBase* detour) {
        auto const loadScriptFunctions = detour->GetTrampolineT<LoadScriptFunctionsT>();
        loadScriptFunctions();

        // register our own lua functions
        DEBUG_LOG("Registering Custom Lua functions");
        
        char isSpellInRange[] = "IsSpellInRange";
        RegisterLuaFunction(isSpellInRange, reinterpret_cast<uintptr_t*>(Offsets::Script_IsSpellInRange));

        char isSpellUsable[] = "IsSpellUsable";
        RegisterLuaFunction(isSpellUsable, reinterpret_cast<uintptr_t*>(Offsets::Script_IsSpellUsable));

        char getSpellIdForName[] = "GetSpellIdForName";
        RegisterLuaFunction(getSpellIdForName, reinterpret_cast<uintptr_t*>(Offsets::Script_GetSpellIdForName));

        char getSpellNameAndRankForId[] = "GetSpellNameAndRankForId";
        RegisterLuaFunction(getSpellNameAndRankForId, reinterpret_cast<uintptr_t*>(Offsets::Script_GetSpellNameAndRankForId));

        char getAndrgitWoWModVersion[] = "GetAndrgitWoWModVersion";
        RegisterLuaFunction(getAndrgitWoWModVersion, reinterpret_cast<uintptr_t*>(Offsets::Script_GetAndrgitWoWModVersion));

        char getDistanceBetween[] = "AWM_GetDistanceBetween";
        RegisterLuaFunction(getDistanceBetween, reinterpret_cast<uintptr_t*>(Offsets::Script_GetDistanceBetween));
    }

    std::once_flag loadFlag;

    void load() {
        std::call_once(loadFlag, []() {
            const hadesmem::Process process(::GetCurrentProcessId());
            
            // hook spell visuals initialize
            auto const spellVisualsInitOrig = hadesmem::detail::AliasCast<SpellVisualsInitializeT>(
                Offsets::SpellVisualsInitialize);
            gSpellVisualsInitDetour = std::make_unique<hadesmem::PatchDetour<SpellVisualsInitializeT >>(process,
                spellVisualsInitOrig,
                &SpellVisualsInitializeHook);
            gSpellVisualsInitDetour->Apply();

            auto const loadScriptFunctionsOrig = hadesmem::detail::AliasCast<LoadScriptFunctionsT>(
                Offsets::LoadScriptFunctions);
            gLoadScriptFunctionsDetour = std::make_unique<hadesmem::PatchDetour<LoadScriptFunctionsT >>(process,
                loadScriptFunctionsOrig,
                &LoadScriptFunctionsHook);
            gLoadScriptFunctionsDetour->Apply();
        });
    }

    void unload() {
        nameplateUnload();

        // Close debug
        if (AndrgitWoWMod::debugLogFile.is_open()) {
            AndrgitWoWMod::debugLogFile.close();
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        AndrgitWoWMod::load();
        break;

    case DLL_PROCESS_DETACH:
        AndrgitWoWMod::unload();
        break;
    }
    return TRUE;
}
