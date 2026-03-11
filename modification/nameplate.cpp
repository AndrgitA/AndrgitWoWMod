#include "nameplate.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "offsets.hpp"



namespace AndrgitWoWMod {
    std::unique_ptr<hadesmem::PatchDetour<NameplateConstructorT>> gNameplateConstructorDetour;
    std::unique_ptr<hadesmem::PatchDetour<NameplateManagerUpdateT>> gNameplateManagerDetour;
    std::unique_ptr<hadesmem::PatchDetour<NameplateBindT>> gNameplateBindDetour;
    std::unique_ptr<hadesmem::PatchDetour<NameplatePrepareT>> gNameplatePrepareDetour;

    std::unordered_map<std::uint32_t, NameplateState*> gNameplateRegistry;
    float nameplateLerpSpeed = 15.0f;
    int const nameplateInitializeReserve = 1000;

    bool IsNameplate(std::uint32_t pFrame) {
        if (!IsValidPtr(pFrame)) return false;

        std::uint32_t vt1 = *reinterpret_cast<std::uint32_t*>(pFrame);
        std::uint32_t vt2 = *reinterpret_cast<std::uint32_t*>(pFrame + 0x24);

        return (vt1 == static_cast<std::uint32_t>(Offsets::Nameplate_VTable1) &&
            vt2 == static_cast<std::uint32_t>(Offsets::Nameplate_VTable2));
    }

    void InitializeNameplateSystem() {
        gNameplateRegistry.reserve(nameplateInitializeReserve);
        DEBUG_LOG("Nameplate Registry initialize reserve.");
    }

    void CleanupNameplateRegistry() {
        // Используем классический цикл по парам (работает в C++11 и выше)
        for (auto const& pair : gNameplateRegistry) {
            // pair.first — это ключ (uint32_t адрес)
            // pair.second — это значение (указатель на NameplateState)
            delete pair.second;
        }
        gNameplateRegistry.clear();
        DEBUG_LOG("Nameplate Registry cleanup: all states deleted.");
    }

    NameplateState* GetOrCreateState(std::uint32_t pNameplate, std::uint32_t pUnit, uint64_t guid) {
        auto it = gNameplateRegistry.find(pNameplate);
        if (it != gNameplateRegistry.end()) {
            if (it->second->guid != guid) {
                it->second->guid = guid;
                it->second->pUnit = pUnit;
                it->second->isFirstFrame = true;
            }
            return it->second;
        }

        NameplateState* state = new NameplateState();
        state->pNameplate = pNameplate;
        state->pUnit = pUnit;
        state->guid = guid;
        state->isFirstFrame = true;
        gNameplateRegistry[pNameplate] = state;
        return state;
    }

    void nameplateLoad() {
        InitializeNameplateSystem();
    }

    void nameplateUnload() {
        // А) Снимаем хуки (вызываем .reset() для unique_ptr)
        gNameplateManagerDetour.reset();
        gNameplateBindDetour.reset();

        // Б) Очищаем память нашей мапы
        CleanupNameplateRegistry();
    }

    void nameplateInitHooks(const hadesmem::Process& process) {
        // Nameplate_Constructor:
        //try {
        //    //gNameplateConstructorDetour = createHook<NameplateConstructorT>(process, Offsets::Nameplate_Constructor, &OnNameplateCreateHook);
        //    DEBUG_LOG("Hook: Nameplate_Constructor applied successfully.");
        //}
        //catch (const std::exception& e) {
        //    DEBUG_LOG("Hook: Nameplate_Constructor FAILED: " << e.what());
        //}
        // 
        // 
        // Nameplate_Bind:
        try {
            gNameplateBindDetour = createHook<NameplateBindT>(process, Offsets::Nameplate_Bind, &OnNameplateBind);
            DEBUG_LOG("Hook: gNameplateBindDetour applied successfully.");
        }
        catch (const std::exception& e) {
            DEBUG_LOG("Hook: gNameplateBindDetour FAILED: " << e.what());
        }

        // Nameplate_ManagerUpdate:
        try {
            //gNameplateManagerDetour = createHook<NameplateManagerUpdateT>(process, Offsets::Nameplate_ManagerUpdate, &OnNameplateManagerUpdate);
            DEBUG_LOG("Hook: OnNameplateManagerUpdate applied successfully.");
        }
        catch (const std::exception& e) {
            DEBUG_LOG("Hook: OnNameplateManagerUpdate FAILED: " << e.what());
        }

        // Nameplate_Prepare:
        try {
            //gNameplatePrepareDetour = createHook<NameplatePrepareT>(process, Offsets::Nameplate_Prepare, &OnNameplatePrepare);
            DEBUG_LOG("Hook: gNameplatePrepareDetour applied successfully.");
        }
        catch (const std::exception& e) {
            DEBUG_LOG("Hook: gNameplatePrepareDetour FAILED: " << e.what());
        }
    }

    void* OnNameplateCreateHook(hadesmem::PatchDetourBase* detour, void* edx, uint32_t arg1, uint32_t arg2) {
        // --- ПОДРОБНЫЙ ДАМП ВХОДА ---
        DEBUG_LOG(">>> [BEFORE ORIG] NAMEPLATE_INIT");
        DEBUG_LOG("ECX (this/detour): 0x" << std::hex << (uintptr_t)detour);
        DEBUG_LOG("EDX (register):    0x" << std::hex << (uintptr_t)edx);
        DEBUG_LOG("STACK_1 (arg1):    0x" << std::hex << arg1 << " (dec: " << std::dec << arg1 << ")");
        DEBUG_LOG("STACK_2 (arg2):    0x" << std::hex << arg2 << " (dec: " << std::dec << arg2 << ")");
        auto const trampoline = detour->GetTrampolineT<NameplateConstructorT>();
        void* result = trampoline(edx, arg1, arg2);

        if (result == nullptr) return result;
        uintptr_t addr = reinterpret_cast<uintptr_t>(result);

        // 1. Читаем GUID (смещения 314 и 315)
        uint32_t low = *reinterpret_cast<uint32_t*>(addr + 314 * 4);
        uint32_t high = *(uint32_t*)(addr + 315 * 4);

        // 2. Читаем ИМЯ (смещение 319)
        const char* unitName = "NullPtr";
        uintptr_t nameFS = *(uintptr_t*)(addr + 319 * 4);

        if (nameFS > 0x1000) {
            // Читаем указатель на строку текста (смещение 0xC0 внутри FontString)
            const char** pText = reinterpret_cast<const char**>(nameFS + 0xC0);

            // Проверяем, что указатель на текст не нулевой и больше 0x1000
            if (pText && (uintptr_t)*pText > 0x1000) {
                unitName = *pText;
            }
            else {
                unitName = "Empty String";
            }
        }

        // 3. Вывод всех данных в одну строку
        DEBUG_LOG("STABLE SCAN: Nameplate: 0x" << std::hex << addr
            << " | GUID: " << high << ":" << low
            << " | Name: " << unitName);
        // 3. Логируем
        DEBUG_LOG("SUCCESS: Nameplate created at: 0x" << std::hex << result);
        return result;
    }

    void OnNameplateManagerUpdate(hadesmem::PatchDetourBase* detour, int** argList, void* edx, float* coords, void* manager) {
        // 1. ДАМПИМ ВСЁ ДО ВЫЗОВА
        DEBUG_LOG(">>> MANAGER SCAN START <<<");
        DEBUG_LOG("ECX (detour): 0x" << std::hex << (uintptr_t)detour);
        DEBUG_LOG("EDX (dummy):  0x" << std::hex << (uintptr_t)edx);
        DEBUG_LOG("STACK_1 (argList): 0x" << std::hex << (uintptr_t)argList);
        DEBUG_LOG("STACK_2 (coords): 0x" << std::hex << (uintptr_t)coords);
        DEBUG_LOG("STACK_3 (manager): 0x" << std::hex << (uintptr_t)manager);

        // 3. Получаем игрока
        //std::uint64_t playerGuid = game::ClntObjMgrGetActivePlayerGuid();
        //uint64_t playerLongGuid = *reinterpret_cast<uint64_t*>(playerGuid + 0x8);

        //// 1. Получаем GUID моба напрямую из объекта (detour + 0x8)
        //uintptr_t unitPtr = reinterpret_cast<uintptr_t>(detour);
        //uint64_t unitGuid = *reinterpret_cast<uint64_t*>(unitPtr + 0x8);


        //auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(0x00515970);
        //uint64_t targetFullGuid = getGUIDFromName("target");
        //DEBUG_LOG("COMPARE: Player(0x" << playerGuid << ": 0x" << playerLongGuid << ") vs UNIT (0x" << unitGuid << ": 0x" << unitPtr <<") vs TARGET IS : 0x" << targetFullGuid << " Target short: 0x" << targetFullGuid);


        //uintptr_t v8 = *reinterpret_cast<uintptr_t*>(unitPtr + 0xE60);
        //DEBUG_LOG("v8: 0x" << v8);
        //if (v8) {
        //    // Попробуй прочитать GUID отсюда. 
        //    // В WoW объекты начинаются с: 0x0 - vtable, 0x8 - GUID
        //    // Но v8 — это может быть "RenderNode".
        //    // Если это RenderNode, GUID юнита может лежать по смещению:
        //    uint64_t guid = *reinterpret_cast<uint64_t*>(v8 + 0x58); // Типичное смещение для привязанного GUID в нодах
        //    DEBUG_LOG("Possible Nameplate GUID: " << std::hex << guid);
        //}
        //uint32_t targetLow = *reinterpret_cast<uint32_t*>(targetFullGuid);; // Отрезаем Low-часть (например, D
        
        // 2. Берем GUID из неймплейта (то, что ты уже вывел в лог как GUID: 2dbf6a70)
        // Мы знаем из IDA, что неймплейт лежит в detour + 920*4
        //uintptr_t nameplatePtr = *(uintptr_t*)((uintptr_t)detour + 920 * 4);
        //if (nameplatePtr > 0x1000) {
        //    uint32_t plateLow = *(uint32_t*)(nameplatePtr + 314 * 4);

        //    // 3. СРАВНИВАЕМ
        //    if (plateLow == targetLow && targetLow != 0) {
        //        DEBUG_LOG("!!! FOUND TARGET NAMEPLATE !!!" << plateLow << " : " << targetLow);
        //        // ПРОВЕРКА КОНТРОЛЯ: Поднимаем таргет в небо
        //        // coords[1] -= 0.2f; 
        //    }
        //}
        //uintptr_t managerAddr = reinterpret_cast<uintptr_t>(detour);
        //uintptr_t nameplatePtr = *reinterpret_cast<uintptr_t*>(managerAddr + 920 * 4);
        //// 2. ПРОВЕРКА: Если неймплейт существует (адрес > 0x1000)
        //uint64_t targetLow = targetFullGuid;
        //DEBUG_LOG("!!! PLATE: 0x" << nameplatePtr);
        //    
        //if (nameplatePtr > 0x0) {
        //    uintptr_t unitPtr = reinterpret_cast<uintptr_t>(argList);
        //    uint64_t unitGuid2 = *reinterpret_cast<uint64_t*>(unitPtr + 0x8);

        //    // Читаем Low GUID неймплейта (смещение 314 * 4)
        //    uint32_t plateLow = *reinterpret_cast<uint32_t*>(nameplatePtr + 314 * 4);

        //    DEBUG_LOG("!!! PLATE LOW: 0x" << plateLow << " !!! 0x" << unitGuid2);
        //    // 3. СРАВНИВАЕМ (Математика с математикой)
        //    if (unitGuid2 == targetLow && targetLow != 0) {
        //        // ЭТО ТВОЯ ЦЕЛЬ!
        //        DEBUG_LOG("!!! TARGET IDENTIFIED: 0x" << std::hex << unitGuid2 << " !!!");

        //        // Магия: поднимаем таргет чуть выше
        //        // coords[1] -= 0.05f; 
        //    }
        //}

        //DEBUG_LOG("COMPARE: Player(0x" << playerGuid << ": 0x" << playerLongGuid << ") vs UNIT (0x" << unitGuid << ": " << uintunitGuid <<") vs TARGET IS : 0x" << targetFullGuid << " Target short: 0x" << targetFullGuid << ": PLATE : 0x" << nameplatePtr);

        
        // arg1 - это указатель на X и Y
        //if (coords > (float*)0x1000) {
        //    float x = coords[0];
        //    float y = coords[1];
        //    DEBUG_LOG("REAL COORDS: X=" << x << " Y=" << y);

        //    // ХОЧЕШЬ ТЕСТ? Подними все неймплейты в небеса:
        //    // arg1[1] -= 200.0f; 
        //}

        

        auto const trampoline = detour->GetTrampolineT<NameplateManagerUpdateT>();
        trampoline(argList, edx, coords, manager);

        // 2. Теперь объект ГАРАНТИРОВАННО создан. Берем его базу:
        // ArgList — это база (ESI в IDA). Смещение 0xE60 — это указатель на Nameplate.
        uintptr_t basePtr = reinterpret_cast<uintptr_t>(argList);
        uintptr_t pNameplate = *reinterpret_cast<uintptr_t*>(basePtr + 0xE60);
        DEBUG_LOG("pNameplate: " << std::hex << pNameplate);

        if (pNameplate > 0x1000) {
            // Мы видели в sub_7CB6D0: mov [esi+4E8h], ecx (Low GUID)
            // и mov [esi+4ECh], edx (High GUID)
            uint64_t unitGuid = *reinterpret_cast<uint64_t*>(pNameplate + 0x4E8);

            if (unitGuid != 0) {
                DEBUG_LOG("STABLE GUID FOUND: " << std::hex << unitGuid);

                // Получаем гильдию (0x005211B0)
                typedef char* (__cdecl* GetGuildNameT)(uint64_t guid);
                auto getGuildName = reinterpret_cast<GetGuildNameT>(0x005211B0);

                char* guildName = getGuildName(unitGuid);
                if (guildName && (uintptr_t)guildName > 0x1000) {
                    DEBUG_LOG("GUILD: " << guildName);
                }
            }
        }
        else {
            DEBUG_LOG("ERROR: pNameplate still 0 after origin call");
        }
        DEBUG_LOG(">>> MANAGER SCAN END <<<");
    }

    //void OnNameplateBind(hadesmem::PatchDetourBase* detour, void* arg1, void* arg2, void* arg3, void* arg4) {
    //    // 1. ДАМПИМ ВСЁ ДО ВЫЗОВА
    //    DEBUG_LOG(">>> OnNameplateBind <<<");
    //    DEBUG_LOG("ECX (detour): 0x" << std::hex << (uintptr_t)detour);
    //    DEBUG_LOG("EDX (arg1):  0x" << std::hex << (uintptr_t)arg1);
    //    DEBUG_LOG("STACK_1 (arg2): 0x" << std::hex << (uintptr_t)arg2);
    //    DEBUG_LOG("STACK_2 (arg3): 0x" << std::hex << (uintptr_t)arg3);
    //    DEBUG_LOG("STACK_3 (arg4): 0x" << std::hex << (uintptr_t)arg4);

    //    auto const trampoline = detour->GetTrampolineT<NameplateBindT>();
    //    trampoline(arg1, arg2, arg3, arg4);

    //    // 2. Логирование входа (arg1 = Nameplate, arg2 = Unit)
    //    uintptr_t pUnit = reinterpret_cast<uintptr_t>(arg3);
    //    DEBUG_LOG("STEP 1: pUnit (arg2) = 0x" << std::hex << pUnit);

    //    if (pUnit > 0x10000) {
    //        // 3. Читаем GUID по подтвержденному смещению +0x30
    //        uint64_t unitGuid = *reinterpret_cast<uint64_t*>(pUnit + 0x30);
    //        DEBUG_LOG("STEP 2: unitGuid (from pUnit + 0x30) = 0x" << std::hex << unitGuid);

    //        if (unitGuid != 0) {
    //            // 4. Сравнение с текущим таргетом
    //            auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(0x00515970);
    //            uint64_t targetGuid = getGUIDFromName("target");
    //            DEBUG_LOG("STEP 3: targetGuid (target) = 0x" << std::hex << targetGuid);

    //            if (unitGuid == targetGuid) {
    //                DEBUG_LOG("!!! BIND MATCH SUCCESS !!! [0x" << std::hex << unitGuid << "] == [0x" << targetGuid << "]");
    //            }
    //            else {
    //                DEBUG_LOG("INFO: New Nameplate created for GUID: 0x" << std::hex << unitGuid);
    //            }
    //        }
    //        else {
    //            DEBUG_LOG("STEP 2 FAIL: unitGuid is 0");
    //        }
    //    }
    //    else {
    //        DEBUG_LOG("STEP 1 FAIL: pUnit is invalid");
    //    }
    //}

    void OnNameplateBind(hadesmem::PatchDetourBase* detour, uint32_t pNameplate, void* edx, uint32_t pUnit) {
        //pNameplate,        // ECX (this)
        //edx,               // EDX (dummy)
        //pUnit,             // Stack 1 (ArgList)
        //unused)            // Stack 2 (остаток стека)
        //    // 1. ДАМПИМ ВСЁ ДО ВЫЗОВА
        //DEBUG_LOG(">>> OnNameplateBind <<<");
        //DEBUG_LOG("ECX (detour): 0x" << std::hex << (uintptr_t)detour);
        //DEBUG_LOG("EDX (arg1):  0x" << std::hex << (uintptr_t)pNameplate);
        //DEBUG_LOG("STACK_1 (arg2): 0x" << std::hex << (uintptr_t)edx);
        //DEBUG_LOG("STACK_2 (arg3): 0x" << std::hex << (uintptr_t)pUnit);
        // 1. Трамплин (вызываем оригинал сразу, чтобы игра привязала данные)
        auto const trampoline = detour->GetTrampolineT<NameplateBindT>();
        trampoline(pNameplate, edx, pUnit);

        //DEBUG_LOG(">>> OnNameplateBind (Verified) <<<");
        //DEBUG_LOG("pNameplate: 0x" << std::hex << pNameplate);
        //DEBUG_LOG("pUnit: 0x" << std::hex << pUnit);

        // 3. Валидация и фильтрация
        if (IsValidPtr(pNameplate) && IsValidPtr(pUnit)) {

            // Наша двойная проверка по VTable
            if (IsNameplate(pNameplate)) {

                // 4. Читаем GUID по подтвержденному смещению 0x30
                uint64_t unitGuid = *reinterpret_cast<uint64_t*>(pUnit + NameplateOffsets::Unit_Guid);
                //DEBUG_LOG("STEP 2: unitGuid (0x30) = 0x" << std::hex << unitGuid);

                if (unitGuid != 0) {
                    // 5. РЕГИСТРАЦИЯ В НАШЕЙ СИСТЕМЕ
                    // Это создаст запись в мапе или обновит её, если фрейм переиспользован
                    NameplateState* state = GetOrCreateState(pNameplate, pUnit, unitGuid);

                    //if (state) {
                    //    DEBUG_LOG("STEP 3: State registered/updated. Map size: " << std::dec << gNameplateRegistry.size());
                    //}

                    // 6. Сравнение с таргетом (для дебага)
                    //auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(0x00515970);
                    //uint64_t targetGuid = getGUIDFromName("target");

                    //if (unitGuid == targetGuid) {
                    //    DEBUG_LOG("!!! BIND MATCH SUCCESS (TARGET) !!!");
                    //}
                }
            }
            //else {
            //    DEBUG_LOG("INFO: Object at 0x" << std::hex << pNameplate << " is NOT a Nameplate (VTable mismatch)");
            //}
        }
        //else {
        //    DEBUG_LOG("ERROR: Invalid Pointers in Bind");
        //}
    }

    void OnNameplatePrepare(hadesmem::PatchDetourBase* detour, void* arg1, void* arg2, void* arg3) {
        // 1. ДАМПИМ ВСЁ ДО ВЫЗОВА
        DEBUG_LOG(">>> OnNameplatePrepare <<<");
        DEBUG_LOG("detour: 0x" << std::hex << (uintptr_t)detour);
        DEBUG_LOG("arg1:  0x" << std::hex << (uintptr_t)arg1);
        DEBUG_LOG("arg2: 0x" << std::hex << (uintptr_t)arg2);
        DEBUG_LOG("arg3: 0x" << std::hex << (uintptr_t)arg3);

        auto const trampoline = detour->GetTrampolineT<NameplatePrepareT>();
        trampoline(arg1, arg2, arg3);

        // 2. Начало логики распознавания (arg1 — это наш pUnit)
        uintptr_t pUnit = reinterpret_cast<uintptr_t>(arg1);
        DEBUG_LOG("STEP 1: pUnit (arg1) = 0x" << std::hex << pUnit);

        if (pUnit > 0x10000) {
            // 3. Чтение GUID из Nameplate по подтвержденному смещению
            uint64_t unitGuid = *reinterpret_cast<uint64_t*>(pUnit + 0x30);
            DEBUG_LOG("STEP 2: unitGuid (from +0x30) = 0x" << std::hex << unitGuid);

            if (unitGuid != 0) {
                // 4. Получение GUID текущего таргета через твой метод
                auto const getGUIDFromName = reinterpret_cast<GetGUIDFromNameT>(0x00515970);
                uint64_t targetGuid = getGUIDFromName("target");

                DEBUG_LOG("STEP 3: targetGuid (from 0x00515970) = 0x" << std::hex << targetGuid);

                // 5. Финальное сравнение
                if (unitGuid == targetGuid) {
                    DEBUG_LOG("!!! MATCH SUCCESS !!! [0x" << std::hex << unitGuid << "] == [0x" << targetGuid << "]");
                }
                else {
                    DEBUG_LOG("INFO: Scanning... Nameplate 0x" << std::hex << unitGuid << " is NOT your target.");
                }
            }
            else {
                DEBUG_LOG("STEP 2 FAIL: unitGuid is 0");
            }
        }
        else {
            DEBUG_LOG("STEP 1 FAIL: pUnit is invalid");
        }
    }
}