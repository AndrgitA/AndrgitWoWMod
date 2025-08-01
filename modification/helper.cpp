//
// Created by pmacc on 9/21/2024.
//

#include "game.hpp"
#include "helper.hpp"
#include "offsets.hpp"
#include "main.hpp"

namespace AndrgitWoWMod {
    char *ConvertGuidToString(uint64_t guid) {
        char *guidStr = new char[21]; // 2 for 0x prefix, 18 for the number, and 1 for '\0'
        std::snprintf(guidStr, 21, "0x%016llX", static_cast<unsigned long long>(guid));
        return guidStr;
    }

    float GetNameplateDistance() {
        auto const distanceSquared = *reinterpret_cast<float *>(Offsets::NameplateDistance);
        return sqrtf(distanceSquared);
    }

    void SetNameplateDistance(float distance) {
        *reinterpret_cast<float *>(Offsets::NameplateDistance) = distance * distance;
    }
    
    float GetDistanceBetweenUnits(const game::C3Vector& pos1, const game::C3Vector& pos2) {
        game::C3Vector v = {};
        v.x = pos1.x - pos2.x;
        v.y = pos1.y - pos2.y;
        v.z = pos1.z - pos2.z;
        return vectorLength(v);
    }

    float GetDistanceBetweenUnits(char* unit1Name, char* unit2Name, DISTANCE_METER meter) {
        if (strlen(unit1Name) == 0 || strlen(unit2Name) == 0) {
            return -1.0f;
        }

        auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(Offsets::GetGUIDFromName);
        uint64_t unit1GUID = 0, unit2GUID = 0;

        if (strncmp(unit1Name, "0x", 2) == 0) {
            unit1GUID = std::stoull(unit1Name, nullptr, 16);
        } else {
            unit1GUID = getGUIDFromName(unit1Name);
        }

        if (unit1GUID == 0) {
            return -1.0f;
        }

        if (strncmp(unit2Name, "0x", 2) == 0) {
            unit2GUID = std::stoull(unit2Name, nullptr, 16);
        }
        else {
            unit2GUID = getGUIDFromName(unit2Name);
        }

        if (unit2GUID == 0) {
            return -1.0f;
        }

        return GetDistanceBetweenUnits(
            game::GetObjectPtr(unit1GUID),
            game::GetObjectPtr(unit2GUID),
            meter
        );
    }

    float GetDistanceBetweenUnits(uintptr_t* unit1_orig, uintptr_t* unit2_orig, DISTANCE_METER meter) {
        if (!unit1_orig || !unit2_orig) {
            return -1;
        }

        uint32_t unit1 = reinterpret_cast<uint32_t>(unit1_orig);
        uint32_t unit2 = reinterpret_cast<uint32_t>(unit2_orig);

        if ((unit1 & 1) != 0 || (unit2 & 1) != 0) {
            return -1;
        }

        if (game::GetObjectPtrType(unit1) != game::OBJECT_TYPE_ID::ID_UNIT &&
            game::GetObjectPtrType(unit1) != game::OBJECT_TYPE_ID::ID_PLAYER) {
            return -1;
        }
        if (game::GetObjectPtrType(unit2) != game::OBJECT_TYPE_ID::ID_UNIT &&
            game::GetObjectPtrType(unit2) != game::OBJECT_TYPE_ID::ID_PLAYER) {
            return -1;
        }

        game::C3Vector pos1 = game::GetUnitPosition(unit1);
        game::C3Vector pos2 = game::GetUnitPosition(unit2);

        // We are ignoring error from game::GetUnitCombatReach
        float combatReach1 = max(0.0f, game::GetUnitCombatReach(unit1));
        float combatReach2 = max(0.0f, game::GetUnitCombatReach(unit2));


        if (meter == DISTANCE_METER::METER_MELEE_AUTOATTACK && abs(pos1.z - pos2.z) < 6.0f) {
            // Melee distance calculation is following https://github.com/vmangos/core/blob/4aaec500a70d32e1234010e432e87982f6e4a527/src/game/Objects/Unit.cpp#L10526

            combatReach1 = max(1.5f, combatReach1);
            combatReach2 = max(1.5f, combatReach2);

            float totalReach = max(5.0f, combatReach1 + combatReach2 + 1.333333373069763f);

            game::C3Vector v = {};
            v.x = pos1.x - pos2.x;
            v.y = pos1.y - pos2.y;

            return max(0.0f, vectorLength(v) - totalReach);
        } else if (meter == DISTANCE_METER::METER_AOE) {
            // AoE distance is following Balake's fix https://github.com/vmangos/core/commit/fc0d6cfd6192b5c90072d77ab289f165ea540a00

            // only 1 reach would be used
            float totalReach = 0.0f;

            // By Balake: testing on classic shows aoe range is bigger vs mob compared to vs player
            // this probably means combat reach is not used vs player targets
            if (game::GetObjectPtrType(unit1) == game::OBJECT_TYPE_ID::ID_UNIT) {
                totalReach = combatReach1;
            }
            if (game::GetObjectPtrType(unit2) == game::OBJECT_TYPE_ID::ID_UNIT) {
                totalReach = combatReach2;
            }

            return max(0.0f, GetDistanceBetweenUnits(pos1, pos2) - totalReach);
        } else if (meter == DISTANCE_METER::METER_CHAINS) {
            // Chains distance is following https://github.com/vmangos/core/blob/9f099e58be8e97dd6ee8215f18feb9ab65b5958c/src/game/Spells/Spell.cpp#L2655

            // We are ignoring error from GetUnitBoundingRadius()
            float boundingRadius1 = max(0.0f, game::GetUnitBoundingRadius(unit1));
            float boundingRadius2 = max(0.0f, game::GetUnitBoundingRadius(unit2));

            return max(0.0f, GetDistanceBetweenUnits(pos1, pos2) - boundingRadius1 - boundingRadius2);
        } else if (meter == DISTANCE_METER::METER_RANGED) {
            return max(0.0f, GetDistanceBetweenUnits(pos1, pos2) - combatReach1 - combatReach2);
        } else {
            // Default to METER_GAUSSIAN
            // While in-DLL we default to METER_GAUSSIAN, for Lua we default to METER_RANGED
            return GetDistanceBetweenUnits(pos1, pos2);
        }
    }

    float vectorLength(const game::C3Vector& vec) {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    }
}