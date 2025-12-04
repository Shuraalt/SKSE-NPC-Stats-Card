#include "Settings.h"

#include <Windows.h>

void Settings::Load() {
    constexpr auto path = L"Data/SKSE/Plugins/StatsCard.ini";

    wchar_t buffer[256];

    //
    // 1) HOTKEY
    //
    DWORD result = GetPrivateProfileStringW(L"Hotkey", L"ScanCode", L"22", buffer, 256, path);
    hotkeyCode = static_cast<std::uint32_t>(_wtoi(buffer));

    //
    // 2) FLIP HOTKEY
    //
    result = GetPrivateProfileStringW(L"Hotkey", L"FlipScanCode", L"8", buffer, 256, path);
    flipHotkeyCode = static_cast<std::uint32_t>(_wtoi(buffer));

    //
    // 3) MODIFIER KEY
    //
    result = GetPrivateProfileStringW(L"Hotkey", L"ModifierKey", L"0", buffer, 256, path);
    modifierKey = _wtoi(buffer);

    //
    // 4) POSITION X
    //
    result = GetPrivateProfileStringW(L"Position", L"X", L"1400", buffer, 256, path);
    posX = _wtoi(buffer);

    //
    // 5) POSITION Y
    //
    result = GetPrivateProfileStringW(L"Position", L"Y", L"200", buffer, 256, path);
    posY = _wtoi(buffer);

    //
    // 6) SCALE
    //
    result = GetPrivateProfileStringW(L"Position", L"Scale", L"100", buffer, 256, path);
    scale = _wtoi(buffer);

    //
    // 7) CARD FLIP BEHAVIOR
    //
    result = GetPrivateProfileStringW(L"Behavior", L"ResetCardOnOpen", L"0", buffer, 256, path);
    resetCardOnOpen = _wtoi(buffer);

    //
    // 8) ENABLE BACK CARD
    //
    result = GetPrivateProfileStringW(L"Behavior", L"EnableBackCard", L"1", buffer, 256, path);
    enableBackCard = _wtoi(buffer);

    //
    // 9) ENABLE SOUND EFFECTS
    //
    result = GetPrivateProfileStringW(L"Audio", L"EnableSoundEffects", L"1", buffer, 256, path);
    enableSoundEffects = _wtoi(buffer);

    //
    // 10) LANGUAGE
    //
    result = GetPrivateProfileStringW(L"Localization", L"Language", L"StatsCard_en", buffer, 256, path);
    // Convert wchar_t to std::string
    char languageBuf[256];
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, languageBuf, 256, nullptr, nullptr);
    language = std::string(languageBuf);

    //
    // Log results
    //
    logger::info("StatsCard.ini loaded:");
    logger::info("  Hotkey = {}", hotkeyCode);
    logger::info("  FlipHotkey = {}", flipHotkeyCode);

    std::string modifierName;
    switch (modifierKey) {
        case 1:
            modifierName = "Alt";
            break;
        case 2:
            modifierName = "Ctrl";
            break;
        case 3:
            modifierName = "Shift";
            break;
        default:
            modifierName = "None";
            break;
    }
    logger::info("  ModifierKey = {} ({})", modifierKey, modifierName);

    logger::info("  X = {}", posX);
    logger::info("  Y = {}", posY);
    logger::info("  Scale = {}", scale);
    logger::info("  ResetCardOnOpen = {} ({})", resetCardOnOpen,
                 resetCardOnOpen == 0 ? "Remember state" : "Always reset to front");
    logger::info("  EnableBackCard = {} ({})", enableBackCard, enableBackCard == 1 ? "Enabled" : "Disabled");
    logger::info("  EnableSoundEffects = {} ({})", enableSoundEffects,
                 enableSoundEffects == 1 ? "Enabled" : "Disabled");
    logger::info("  Language = {}", language);
}