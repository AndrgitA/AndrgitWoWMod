//
// Created by pmacc on 1/8/2025.
//

#include "scripts.hpp"
#include "offsets.hpp"
#include "helper.hpp"

namespace AndrgitWoWMod {
    auto const lua_error = reinterpret_cast<lua_errorT>(Offsets::lua_error);

    auto const lua_isstring = reinterpret_cast<lua_isstringT>(Offsets::lua_isstring);
    auto const lua_isnumber = reinterpret_cast<lua_isnumberT>(Offsets::lua_isnumber);

    auto const lua_tostring = reinterpret_cast<lua_tostringT>(Offsets::lua_tostring);
    auto const lua_tonumber = reinterpret_cast<lua_tonumberT>(Offsets::lua_tonumber);

    auto const lua_gettop = reinterpret_cast<lua_gettopT>(Offsets::lua_gettop);
    auto const lua_pushnumber = reinterpret_cast<lua_pushnumberT>(Offsets::lua_pushnumber);
    auto const lua_pushstring = reinterpret_cast<lua_pushstringT>(Offsets::lua_pushstring);

    uint32_t GetSpellIdFromSpellName(const char *spellName) {
        auto const GetSpellSlotAndBookTypeFromSpellName = reinterpret_cast<GetSpellSlotAndBookTypeFromSpellNameT>(Offsets::GetSpellIdFromSpellName);
        uint32_t bookType;
        uint32_t spellSlot = GetSpellSlotAndBookTypeFromSpellName(spellName, &bookType);

        uint32_t spellId = 0;
        if (spellSlot < 1024) {
            if (bookType == 0) {
                spellId = *reinterpret_cast<uint32_t *>(uint32_t(Offsets::CGSpellBook_mKnownSpells) +
                                                        spellSlot * 4);
            } else {
                spellId = *reinterpret_cast<uint32_t *>(uint32_t(Offsets::CGSpellBook_mKnownPetSpells) +
                                                        spellSlot * 4);
            }
        }

        return spellId;
    }

    uint32_t Script_IsSpellInRange(hadesmem::PatchDetourBase *detour, uintptr_t *luaState) {
        auto param1IsString = lua_isstring(luaState, 1);
        auto param1IsNumber = lua_isnumber(luaState, 1);
        if (param1IsString || param1IsNumber) {
            uint32_t spellId = 0;

            if (param1IsNumber) {
                spellId = uint32_t(lua_tonumber(luaState, 1));

                if (spellId == 0) {
                    lua_error(luaState, "Unable to parse spell id");
                    return 0;
                }
            } else {
                auto const spellName = lua_tostring(luaState, 1);

                spellId = GetSpellIdFromSpellName(spellName);
                if (spellId == 0) {
                    lua_error(luaState,
                              "Unable to determine spell id from spell name, possibly because it isn't in your spell book.  Try IsSpellInRange(SPELL_ID) instead");
                    return 0;
                }
            }

            auto spell = game::GetSpellInfo(spellId);
            if (spell) {
                std::set<uint32_t> validTargetTypes = {5, 6, 21, 25};
                if (sizeof spell->EffectImplicitTargetA == 0 ||
                    validTargetTypes.count(spell->EffectImplicitTargetA[0]) == 0) {
                    lua_pushnumber(luaState, -1.0);
                    return 1;
                }

                char *target;

                if (lua_isstring(luaState, 2)) {
                    target = lua_tostring(luaState, 2);
                } else {
                    char defaultTarget[] = "target";
                    target = defaultTarget;
                }

                uint64_t targetGUID;
                if (strncmp(target, "0x", 2) == 0 || strncmp(target, "0X", 2) == 0) {
                    // already a guid
                    targetGUID = std::stoull(target, nullptr, 16);
                } else {
                    auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(Offsets::GetGUIDFromName);
                    targetGUID = getGUIDFromName(target);
                }

                auto playerUnit = game::GetObjectPtr(game::ClntObjMgrGetActivePlayerGuid());

                auto const RangeCheckSelected = reinterpret_cast<RangeCheckSelectedT>(Offsets::RangeCheckSelected);
                auto const result = RangeCheckSelected(playerUnit, spell, targetGUID, '\0');

                if (result != 0) {
                    lua_pushnumber(luaState, 1.0);
                } else {
                    lua_pushnumber(luaState, 0);
                }
                return 1;
            } else {
                lua_error(luaState, "Spell not found");
            }
        } else {
            lua_error(luaState, "Usage: IsSpellInRange(spellName)");
        }

        return 0;
    }

    uint32_t Script_IsSpellUsable(hadesmem::PatchDetourBase *detour, uintptr_t *luaState) {
        auto param1IsString = lua_isstring(luaState, 1);
        auto param1IsNumber = lua_isnumber(luaState, 1);
        if (param1IsString || param1IsNumber) {
            uint32_t spellId = 0;

            if (param1IsNumber) {
                spellId = uint32_t(lua_tonumber(luaState, 1));

                if (spellId == 0) {
                    lua_error(luaState, "Unable to parse spell id");
                    return 0;
                }
            } else {
                auto const spellName = lua_tostring(luaState, 1);

                spellId = GetSpellIdFromSpellName(spellName);
                if (spellId == 0) {
                    lua_error(luaState,
                              "Unable to determine spell id from spell name, possibly because it isn't in your spell book.  Try IsSpellUsable(SPELL_ID) instead");
                    return 0;
                }
            }

            auto spell = game::GetSpellInfo(spellId);
            if (spell) {
                auto const IsSpellUsable = reinterpret_cast<Spell_C_IsSpellUsableT>(Offsets::Spell_C_IsSpellUsable);

                uint32_t outOfMana = 0;
                auto const result = IsSpellUsable(spell, &outOfMana) & 0xFF;

                if (result != 0) {
                    lua_pushnumber(luaState, 1.0);
                } else {
                    lua_pushnumber(luaState, 0);
                }

                if (outOfMana) {
                    lua_pushnumber(luaState, 1.0);
                } else {
                    lua_pushnumber(luaState, 0);
                }

                return 2;
            } else {
                lua_error(luaState, "Spell not found");
            }
        } else {
            lua_error(luaState, "Usage: IsSpellUsable(spellName)");
        }

        return 0;
    }

    uint32_t Script_GetSpellIdForName(hadesmem::PatchDetourBase *detour, uintptr_t *luaState) {
        if (lua_isstring(luaState, 1)) {
            auto const spellName = lua_tostring(luaState, 1);
            auto const spellId = GetSpellIdFromSpellName(spellName);
            lua_pushnumber(luaState, spellId);
            return 1;
        } else {
            lua_error(luaState, "Usage: GetSpellIdForName(spellName)");
        }

        return 0;
    }

    uint32_t Script_GetSpellNameAndRankForId(hadesmem::PatchDetourBase *detour, uintptr_t *luaState) {
        if (lua_isnumber(luaState, 1)) {
            auto const spellId = uint32_t(lua_tonumber(luaState, 1));
            auto const spell = game::GetSpellInfo(spellId);

            if (spell) {
                auto const language = *reinterpret_cast<std::uint32_t *>(Offsets::Language);
                lua_pushstring(luaState, (char *) spell->SpellName[language]);
                lua_pushstring(luaState, (char *) spell->Rank[language]);
                return 2;
            } else {
                lua_error(luaState, "Spell not found");
            }
        } else {
            lua_error(luaState, "Usage: GetSpellNameAndRankForId(spellId)");
        }

        return 0;
    }

    uint32_t Script_GetAndrgitWoWModVersion(hadesmem::PatchDetourBase *detour, uintptr_t *luaState) {
        lua_pushnumber(luaState, MAJOR_VERSION);
        lua_pushnumber(luaState, MINOR_VERSION);
        lua_pushnumber(luaState, PATCH_VERSION);

        return 3;
    }

    uint32_t Script_GetDistanceBetween(hadesmem::PatchDetourBase* detour, uintptr_t* luaState) {
        if (lua_gettop(luaState) < 2) {
            lua_error(luaState, "GetDistanceBetween: Need minimum 2 args");
            return 0;
        }

        auto const unit1Name = lua_tostring(luaState, 1);
        auto const unit2Name = lua_tostring(luaState, 2);
        DISTANCE_METER meter = DISTANCE_METER::METER_RANGED; // While in-DLL we default to METER_GAUSSIAN, for Lua we default to METER_RANGED

        if (lua_gettop(luaState) >= 3) {
            auto const meterName = lua_tostring(luaState, 3);

            if (strncmp(meterName, "meleeAutoAttack", 15) == 0) {
                meter = DISTANCE_METER::METER_MELEE_AUTOATTACK;
            } else if (strncmp(meterName, "AoE", 3) == 0) {
                meter = DISTANCE_METER::METER_AOE;
            } else if (strncmp(meterName, "chains", 6) == 0) {
                meter = DISTANCE_METER::METER_CHAINS;
            } else if (strncmp(meterName, "Gaussian", 8) == 0) {
                meter = DISTANCE_METER::METER_GAUSSIAN;
            }
        }

        lua_pushnumber(luaState, GetDistanceBetweenUnits(unit1Name, unit2Name, meter));
        return 1;
    }
}