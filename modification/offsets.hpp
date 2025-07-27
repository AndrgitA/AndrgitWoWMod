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
    //StartSpellVisualCheck = 0x006E79B5, // [7240117]

    ClntObjMgrObjectPtr = 0x00468460, // [4621408]
    GetObjectPtr = 0x464870, // [4606064]
    GetActivePlayer = 0x468550, // [4621648]
    //GetUnitFromName = 0x00515940, // [5331264]
    GetDurationObject = 0x00C0D828, // [12638248]
    GetCastingTimeIndex = 0x2D, // [45]
    Language = 0xC0E080, // [12640384]
    SpellDb = 0xC0D788, // [12638088]
    //CursorMode = 0xBE2C4C,
    //SpellIsTargeting = 0xCECAC0,
    //CastingItemIdPtr = 0X00CECAB0,
    //CastingSpellId = 0xCECA88,
    //AutoRepeatingSpellId = 0xCEAC30,
    //VisualSpellId = 0X00CEAC58,
    //GetSpellSlotAndType = 0X004B3950,
    //OsGetAsyncTimeMs = 0X0042B790,
    //ChannelTargetGuid = 0xC4D980,
    NameplateDistance = 0xC4D988, // [12900744] float containing the distance squared ready to be pythagorean'd
    //DBCacheGetRecord = 0X0055BA30,

    //ItemDBCache = 0xC0E2A0,

    CGSpellBook_mKnownSpells = 0xB700F0, // [11993328]
    CGSpellBook_mKnownPetSpells = 0xB6F098, // [11989144]
    //CGSpellBook_mKnownAbilities = 0XB712A8,

    //IsSpellInRangeOfUnit = 0X004E56F0,
    //CancelSpell = 0x6E4940,
    //CancelAutoRepeatSpell = 0X006EA080,
    //SpellDelayed = 0x6E74F0,
    //SignalEvent = 0x703E50,
    //SignalEventParam = 0x703F50,
    //ISceneEndPtr = 0x5A17A0,
    //SpellStart = 0X006E7700,
    //SpellGo = 0x006E7A70,
    //CooldownEvent = 0x006E9670,
    //SendCast = 0x6E54F0,
    //CreateCastbar = 0x6E7A53,
    //CheckAndReportSpellInhibitFlags = 0x006094f0,

    LockedTargetGuid = 0x00B4E2D8, // [11854552]
    //OnSpriteRightClick = 0x00492820,
    CGGameUI_Target = 0x00493540, // [4797760]

    //SpellVisualsHandleCastStart = 0X006EC220,
    //PlaySpellVisualHandler = 0X006E98D0,
    //PlaySpellVisual = 0X0060EDF0,

    //SpellChannelStartHandler = 0x006E7550,
    //SpellChannelUpdateHandler = 0x006E75F0,
    //SpellFailedHandler = 0x006E8D80,
    //SpellFailedOtherHandler = 0X006E8E40,
    //CastResultHandler = 0x006E7330,
    //SpellStartHandler = 0x006E7640,
    //SpellCooldownHandler = 0X006E9460,
    //PeriodicAuraLogHandler = 0X00626DD0,
    //SpellNonMeleeDmgLogHandler = 0X005E85E0,

    //Spell_C_GetCastTime = 0X006E3340,
    //Spell_C_CastSpell = 0x6E4B60,
    //Spell_C_CooldownEventTriggered = 0X006E3050,
    //Spell_C_SpellFailed = 0x006E1A00,
    //Spell_C_CastSpellByID = 0x6E5A90,
    //Spell_C_GetAutoRepeatingSpell = 0x006E9FD0,
    //Spell_C_HandleSpriteClick = 0x006E5B10,
    Spell_C_TargetSpell = 0x006E5250, // [7230032]
    //Spell_C_GetSpellCooldown = 0X006E2EA0,
    Spell_C_IsSpellUsable = 0x006E3D60, // [7224672]

    CVarLookup = 0x0063DEC0, // [6545088]
    RegisterCVar = 0x0063DB90, // [6544272]

    //GetClientConnection = 0X005AB490,
    //GetNetStats = 0X00537F20,
    //ClientServicesSendMessagePtr = 0X00C28128,

    LoadScriptFunctions = 0x00490250, // [4784720]
    FrameScript_RegisterFunction = 0x00704120, // [7356704]
    FrameScript_CreateEvents = 0x00703D90, // [7355792]

    //// Existing script functions
    GetGUIDFromName = 0x00515970, // [5331312]
    //Script_CastSpellByName = 0x004B4AB0, // [4934320]
    //Script_SpellTargetUnit = 0x006E6D90, // [7237008]
    Script_SetCVar = 0x00488C10, // [4754448]
    //Script_RunScript = 0x0048B980,
    //Script_SpellStopCasting = 0x006E6E80,

    //// Added script functions
    //Script_QueueSpellByName = 0x004B4B38, // [4934456] unused address at the end of Script_CastSpellByName.  Need to be < 0x7FEDAC [8383916] to avoid Invalid Function Pointer
    //Script_CastSpellByNameNoQueue = 0x004B4B64, // [4934500]
    //Script_QueueScript = 0x0048B968, // [4766056] unused address at the end of Script_StopCinematic
    Script_IsSpellInRange = 0x004E76D8, // [5142232]
    Script_IsSpellUsable = 0x004E77A4, // [5142436]
    //Script_GetCurrentCastingInfo = 0x004E77F8, // [5142520]
    Script_GetSpellIdForName = 0x004E7828, // [5142568]
    Script_GetSpellNameAndRankForId = 0x004E7844, // [5142596]
    //Script_GetSpellSlotTypeIdForName = 0x004E784A, // [5142602]
    //Script_ChannelStopCastingNextTick = 0x004E7858, // [5142616]
    Script_GetAndrgitWoWModVersion = 0x004E7874, // [5142644]
    //Script_GetItemLevel = 0x004E787A, // [5142650]

    //lua_state_ptr = 0x7040D0,

    lua_isstring = 0x006F3510, // [7288080]
    lua_isnumber = 0x006F34D0, // [7288016]
    lua_tostring = 0x006F3690, // [7288464]
    lua_tonumber = 0x006F3620, // [7288352]
    lua_pushnumber = 0x006F3810, // [7288848]
    //lua_gettable = 0x6F3A40, // [7289408]
    lua_pushstring = 0x006F3890, // [7288976]
    //lua_pushnil = 0x006F37F0, // [7288816]
    //lua_call = 0x00704CD0, // [7359696]
    //lua_pcall = 0x006F41A0, // [7291296]
    lua_error = 0x006F4940, // [7293248]
    //lua_settop = 0x006F3080, // [7286912]


    CGInputControlGetActive = 0xBE1148, // [12456264]
    LastHardwareAction = 0x00CF0BC8, // [13568968]
    CGInputControlSetReleaseAction = 0x514810,// [5326864]
    CGInputControlSetControlBit = 0x515090, // [5329040]

    RangeCheckSelected = 0x6E4440, // [7226432]

    GetSpellIdFromSpellName = 0x004B3950, // [4929872]

    SpellVisualsInitialize = 0x006ec0e0, // [7258336]

    //IntIntParamFormat = 0X00843342,
    //StringIntParamFormat = 0X00847FBC,

    //QueueEventStringPtr = 0X00BE175C, // unused event 369 0x171
    //CastEventStringPtr = 0X00BE1A08, // unused event 540 0x21C
    //SpellDamageEventSelfStringPtr = 0X00BE1A2C, // unused event 549 0x225
    //SpellDamageEventOtherStringPtr = 0X00BE1A30, // unused event 550 0x226
};
