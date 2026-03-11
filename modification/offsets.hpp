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

#pragma once

#include <cstdint>

enum class Offsets : std::uint32_t {
    ClntObjMgrObjectPtr = 0x00468460, // [4621408]
    GetObjectPtr = 0x464870, // [4606064]
    GetActivePlayer = 0x468550, // [4621648]
    GetDurationObject = 0x00C0D828, // [12638248]
    GetCastingTimeIndex = 0x2D, // [45]
    Language = 0xC0E080, // [12640384]
    SpellDb = 0xC0D788, // [12638088]
    NameplateDistance = 0xC4D988, // [12900744] float containing the distance squared ready to be pythagorean'd

    CGSpellBook_mKnownSpells = 0xB700F0, // [11993328]
    CGSpellBook_mKnownPetSpells = 0xB6F098, // [11989144]

    LockedTargetGuid = 0x00B4E2D8, // [11854552]
    CGGameUI_Target = 0x00493540, // [4797760]

    Spell_C_TargetSpell = 0x006E5250, // [7230032]
    Spell_C_IsSpellUsable = 0x006E3D60, // [7224672]

    CVarLookup = 0x0063DEC0, // [6545088]
    RegisterCVar = 0x0063DB90, // [6544272]

    LoadScriptFunctions = 0x00490250, // [4784720]
    FrameScript_RegisterFunction = 0x00704120, // [7356704]
    FrameScript_CreateEvents = 0x00703D90, // [7355792]

    //// Existing script functions
    GetGUIDFromName = 0x00515970, // [5331312]
    Script_SetCVar = 0x00488C10, // [4754448]

    //// Added script functions
    Script_IsSpellInRange = 0x004E76D8, // [5142232]
    Script_IsSpellUsable = 0x004E77A4, // [5142436]
    Script_GetSpellIdForName = 0x004E7828, // [5142568]
    Script_GetSpellNameAndRankForId = 0x004E7844, // [5142596]
    Script_GetAndrgitWoWModVersion = 0x004E7874, // [5142644]

    //lua_state_ptr = 0x7040D0,

    lua_isstring = 0x006F3510, // [7288080]
    lua_isnumber = 0x006F34D0, // [7288016]
    lua_tostring = 0x006F3690, // [7288464]
    lua_tonumber = 0x006F3620, // [7288352]
    lua_gettop = 0x006F3070, // [7286896]
    lua_pushnumber = 0x006F3810, // [7288848]
    lua_pushstring = 0x006F3890, // [7288976]
    lua_error = 0x006F4940, // [7293248]

    CGInputControlGetActive = 0xBE1148, // [12456264]
    LastHardwareAction = 0x00CF0BC8, // [13568968]
    CGInputControlSetReleaseAction = 0x514810,// [5326864]
    CGInputControlSetControlBit = 0x515090, // [5329040]

    RangeCheckSelected = 0x6E4440, // [7226432]

    GetSpellIdFromSpellName = 0x004B3950, // [4929872]

    SpellVisualsInitialize = 0x006ec0e0, // [7258336]

    Script_GetDistanceBetween = 0x004e784c, // [5142604]

    //testing functional
    Nameplate_Constructor = 0x007CB250, // 8172112
    Nameplate_ManagerUpdate = 0x006086E0, // 6325984
    Nameplate_Bind = 0x007CB6D0, // 8173264
    Nameplate_Prepare = 0x0060F600, // 6354432

    // --- Системные адреса (Функции) ---
    CSimpleFrame_Hide = 0x0076AD50, // vtable +132
    CSimpleFrame_Show = 0x0076AE10, // vtable +136

    // --- Таблицы функций (VTable) ---
    Nameplate_VTable1 = 0x0081DE50,  // +0x0
    Nameplate_VTable2 = 0x0081DE24  // +0x24
};
