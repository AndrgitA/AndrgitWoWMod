//
// Created by pmacc on 9/21/2024.
//

#include "spellcast.hpp"
//#include "helper.hpp"
#include "offsets.hpp"
#include "logging.hpp"

namespace AndrgitWoWMod {
    void SetReleaseAction(uint32_t input) {
        uint32_t activeControl = *reinterpret_cast<uint32_t *>(Offsets::CGInputControlGetActive);

        typedef void(__thiscall *SetReleaseActionT)(uint32_t, uint32_t);
        auto SetReleaseAction = reinterpret_cast<SetReleaseActionT>(Offsets::CGInputControlSetReleaseAction);
        SetReleaseAction(activeControl, input);
    }

    void SetControlBit(uint32_t input) {
        uint32_t activeControl = *reinterpret_cast<uint32_t *>(Offsets::CGInputControlGetActive);
        auto *LastHardwareAction = reinterpret_cast<uintptr_t *>(Offsets::LastHardwareAction);

        typedef void(__thiscall *SetControlBitT)(uint32_t, uint32_t, uint32_t, uintptr_t *, int);
        auto SetControlBit = reinterpret_cast<SetControlBitT>(Offsets::CGInputControlSetControlBit);
        SetControlBit(activeControl, 2, input, LastHardwareAction, 0);
    }

    void CameraOrSelectOrMoveStart() {
        SetReleaseAction(1);
        SetControlBit(1);
    }

    void CameraOrSelectOrMoveStop() {
        SetControlBit(0);
    }

    bool Spell_C_TargetSpellHook(hadesmem::PatchDetourBase *detour,
                                 uint32_t *player,
                                 uint32_t *spellId,
                                 uint32_t unk3,
                                 float unk4) {
        auto const spellTarget = detour->GetTrampolineT<Spell_C_TargetSpellT>();
        auto result = spellTarget(player, spellId, unk3, unk4);

        if (!result) {
            auto const spellName = game::GetSpellName(*spellId);
            auto const spell = game::GetSpellInfo(*spellId);

            if (spell->Targets == game::SpellTarget::TARGET_LOCATION_UNIT_POSITION &&
                spell->Effect[0] != game::SPELL_EFFECT_SUMMON_GUARDIAN) {
                // if quickcast is on instantly trigger all casts
                // otherwise if this is a queued cast, trigger it instant cast
                if (gUserSettings.quickcastTargetingSpells) {
                    DEBUG_LOG("Quickcasting terrain spell " << spellName
                                                            << " quickcast: "
                                                            << gUserSettings.quickcastTargetingSpells);

                    // store the current target
                    auto const targetGuid = game::GetCurrentTargetGuid();

                    CameraOrSelectOrMoveStart();
                    CameraOrSelectOrMoveStop();

                    // check if target changed
                    if (targetGuid != game::GetCurrentTargetGuid()) {
                        DEBUG_LOG("Target changed during quick cast, restoring previous target " << targetGuid);
                        auto const targetUnit = reinterpret_cast<CGGameUI_TargetT>(Offsets::CGGameUI_Target);
                        targetUnit(targetGuid);
                    }
                }
            }
        }
        return result;
    }
}