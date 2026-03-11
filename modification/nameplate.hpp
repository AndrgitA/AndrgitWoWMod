#pragma once

#include "main.hpp"
#include <unordered_map>

//// GUID юнита (314 и 315 DWORD-ы)
//static const uintptr_t UnitGuidLow = 314 * 4; // 0x4E8
//static const uintptr_t UnitGuidHigh = 315 * 4; // 0x4EC

//// Указатели на дочерние UI-объекты
//static const uintptr_t HealthBarPtr = 316 * 4; // 0x4F0 (StatusBar)
//static const uintptr_t NameTextPtr = 319 * 4; // 0x4FC (FontString)
//static const uintptr_t LevelTextPtr = 320 * 4; // 0x500 (FontString)

//// Текст внутри FontString (смещение внутри самого объекта FontString)
//static const uintptr_t FontStringText = 0xC0;
namespace NameplateOffsets {
    // CUnit (OnNameplatePrepare)
    static const std::uint32_t Unit_Guid = 0x30;

    // Nameplate (CStatusFrame)
    static const std::uint32_t Nameplate_Guid = 0x4E8;      // Unit GUID
    static const std::uint32_t Nameplate_isShown = 0xD0;    // Устанавливается при парсинге/скриптах
    static const std::uint32_t Nameplate_IsVisible = 0xD4;  // Flag m_shown (bool)
    static const std::uint32_t Nameplate_X = 0x508;         // float (1288 byte)
    static const std::uint32_t Nameplate_Y = 0x50C;         // float (1292 byte)
}

namespace AndrgitWoWMod {
    using NameplateConstructorT = void* (__fastcall*)(void* edx, uint32_t arg1, uint32_t arg2);
    using NameplateManagerUpdateT = void(__fastcall*)(int** argList, void* edx, float* coords, void* manager);
    using NameplateBindT = void(__fastcall*)(uint32_t pNameplate, void* edx, uint32_t pUnit);
    using NameplatePrepareT = void(__fastcall*)(void* arg1, void* arg2, void* arg3);

    void nameplateInitHooks(const hadesmem::Process& process);
    void nameplateLoad();
    void nameplateUnload();

    struct NameplateState {
        std::uint32_t pNameplate;
        std::uint32_t pUnit;
        uint64_t guid;

        // coordinates for Lerp
        float targetX, targetY;   // original
        float currentX, currentY; // real

        bool isFirstFrame = true;
        uint32_t lastUpdate = 0;  // GetTime()
    };

    extern float nameplateLerpSpeed;
    extern std::unordered_map<uintptr_t, NameplateState*> gNameplateRegistry;

    // Проверка типа объекта
    bool IsNameplate(std::uint32_t pFrame);

    void InitializeNameplateSystem();
    void CleanupNameplateRegistry();
    NameplateState* GetOrCreateState(std::uint32_t pNameplate, std::uint32_t pUnit, uint64_t guid);

	// Сигнатура должна точно совпадать с реализацией
    void OnNameplateBind(hadesmem::PatchDetourBase* detour, uint32_t pNameplate, void* edx, uint32_t pUnit);


	void* OnNameplateCreateHook(hadesmem::PatchDetourBase* detour, void* edx, uint32_t arg1, uint32_t arg2);
    void OnNameplateManagerUpdate(hadesmem::PatchDetourBase* detour, int** argList, void* edx, float* coords, void* manager);
    void OnNameplatePrepare(hadesmem::PatchDetourBase* detour, void* arg1, void* arg2, void* arg3);
}