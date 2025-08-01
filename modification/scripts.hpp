//
// Created by pmacc on 1/8/2025.
//

#pragma once

#include <Windows.h>
#include "main.hpp"

namespace AndrgitWoWMod {
    uint32_t Script_IsSpellInRange(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);

    uint32_t Script_GetAndrgitWoWModVersion(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);

    uint32_t Script_IsSpellUsable(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);

    uint32_t Script_GetSpellIdForName(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);

    uint32_t Script_GetSpellNameAndRankForId(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);

    uint32_t Script_GetDistanceBetween(hadesmem::PatchDetourBase* detour, uintptr_t* luaState);
}