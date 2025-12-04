#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <string>

class Settings {
public:
    static Settings* GetSingleton() {
        static Settings singleton;
        return &singleton;
    }

    void Load();

    // Hotkey scancode (default 22 = U key)
    std::uint32_t hotkeyCode = 22;

    // Flip card hotkey scancode (default 8 = "7" key)
    std::uint32_t flipHotkeyCode = 8;

    // Modifier keys (0 = none, 1 = Alt, 2 = Ctrl, 3 = Shift)
    // Applies to BOTH hotkeyCode and flipHotkeyCode
    std::int32_t modifierKey = 0;

    std::int32_t posX = 1400;
    std::int32_t posY = 200;
    std::int32_t scale = 100;

    // Card flip behavior
    // 0 = Remember last state (Option A)
    // 1 = Always reset to front (Option B)
    std::int32_t resetCardOnOpen = 0;

    // Enable/disable back card feature
    // 0 = Disabled (no flip, single card only)
    // 1 = Enabled (can flip with 7 key)
    std::int32_t enableBackCard = 1;

    // Sound effects settings
    // 0 = Disabled (no sounds)
    // 1 = Enabled (play UI sounds)
    std::int32_t enableSoundEffects = 1;

    // Language file to load (e.g., "StatsCard_en", "StatsCard_zh")
    std::string language = "StatsCard_en";

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
};