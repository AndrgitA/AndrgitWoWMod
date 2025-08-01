//
// Created by pmacc on 9/21/2024.
//

#pragma once

#include "game.hpp"

namespace AndrgitWoWMod {

    /*bool SpellIsOnGcd(const game::SpellRec *spell);

    bool SpellIsChanneling(const game::SpellRec *spell);

    bool SpellIsTargeting(const game::SpellRec *spell);

    bool SpellIsOnSwing(const game::SpellRec *spell);

    bool SpellIsAttackTradeskillOrEnchant(const game::SpellRec *spell);

    uint32_t GetGcdOrCooldownForSpell(uint32_t spellId);

    uint32_t GetRemainingGcdOrCooldownForSpell(uint32_t spellId);

    uint32_t GetRemainingCooldownForSpell(uint32_t spellId);

    bool IsSpellOnCooldown(uint32_t spellId);

    */

    enum DISTANCE_METER {
        METER_AOE,				// AoE spells. Like novas and whirlwind.
        METER_GAUSSIAN,			// Raw distance. Calculations like camera frustum should be using this meter
        METER_RANGED,			// Ranged, Targeted spells. Like bolts, heals and charge.
        METER_MELEE_AUTOATTACK,	// Melee auto attack. Note this meter isn`t exactly fit into melee spells because of ignoring Z-axis. We could find a spot where mobs could melee us but we out of taunt range
        METER_CHAINS,			// To tell if spell would chain from one to another. Like cleave, multishot. Vmangos CHAIN_SPELL_JUMP_RADIUS is 10
    };

    char* ConvertGuidToString(uint64_t guid);

    float GetNameplateDistance();

    void SetNameplateDistance(float distance);

    float GetDistanceBetweenUnits(const game::C3Vector& pos1, const game::C3Vector& pos2);

    float GetDistanceBetweenUnits(char* unit1Name, char* unit2Name, DISTANCE_METER meter);

    float GetDistanceBetweenUnits(uintptr_t* unit1, uintptr_t* unit2, DISTANCE_METER meter);

    float vectorLength(const game::C3Vector& vec);
}