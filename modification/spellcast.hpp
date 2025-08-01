//
// Created by pmacc on 9/21/2024.
//

#pragma once

#include "game.hpp"
#include <Windows.h>
#include "main.hpp"

namespace AndrgitWoWMod {
    bool Spell_C_TargetSpellHook(hadesmem::PatchDetourBase* detour,
        uint32_t* player,
        uint32_t* spellId,
        uint32_t unk3,
        float unk4);
}