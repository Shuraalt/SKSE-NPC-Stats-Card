// File: plugin.cpp

#include <ShlObj.h>
#include <Windows.h>  // Required for GetAsyncKeyState
#include <spdlog/sinks/basic_file_sink.h>

#include "LocalizationManager.h"
#include "PCH.h"
#include "Settings.h"
#include "StatsCardMenu.h"
#include "TrueDirectionalMovementAPI.h"

static TDM_API::IVTDM1* g_tdm = nullptr;

// ---------------- SOUND HELPER --------------------
static void PlayUISound(const char* editorID) {
    // Check if sound effects are enabled in settings
    auto settings = Settings::GetSingleton();
    if (settings->enableSoundEffects == 0) {
        return;  // Sounds disabled, do nothing
    }

    auto audioManager = RE::BSAudioManager::GetSingleton();
    if (!audioManager) {
        logger::warn("BSAudioManager not available");
        return;
    }

    RE::BSSoundHandle soundHandle;
    audioManager->BuildSoundDataFromEditorID(soundHandle, editorID, 0x1A);

    if (soundHandle.IsValid()) {
        soundHandle.Play();
    } else {
        logger::warn("Failed to play sound: {}", editorID);
    }
}

// ---------------- TDM INIT (for targeting) --------------------
static void InitTDMAPI() {
    void* api = TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1);
    if (api) {
        g_tdm = reinterpret_cast<TDM_API::IVTDM1*>(api);
        logger::info("TDM API acquired");
    } else {
        logger::warn("TDM API not available");
    }
}

// ---------------- TARGET ACQUISITION ---------------------

// 1) TDM current target (hard lock / soft lock)
static RE::Actor* GetCurrentTDMTarget() {
    if (!g_tdm) {
        return nullptr;
    }

    auto handle = g_tdm->GetCurrentTarget();
    if (!handle) {
        return nullptr;
    }

    auto actor = handle.get().get();
    return actor ? actor->As<RE::Actor>() : nullptr;
}

// 2) Crosshair fallback
static RE::Actor* GetCrosshairTarget() {
    auto cross = RE::CrosshairPickData::GetSingleton();
    if (!cross) {
        return nullptr;
    }

    auto ref = cross->target.get();
    if (!ref) {
        return nullptr;
    }

    return ref->As<RE::Actor>();
}

// 3) Unified targeting logic
static RE::Actor* GetLockedTargetFromTrueHUD() {
    if (auto tdmTarget = GetCurrentTDMTarget()) {
        return tdmTarget;
    }

    if (auto cs = GetCrosshairTarget()) {
        return cs;
    }

    return nullptr;
}

// Helper function to check if modifier key is pressed
// FIXED: Uses Windows API to avoid linker errors with BSWin32KeyboardDevice
static bool IsModifierKeyPressed(std::int32_t modifierType) {
    if (modifierType == 0) return true;  // No modifier required

    int vk = 0;
    switch (modifierType) {
        case 1:
            vk = VK_MENU;
            break;  // Alt
        case 2:
            vk = VK_CONTROL;
            break;  // Ctrl
        case 3:
            vk = VK_SHIFT;
            break;  // Shift
        default:
            return true;
    }

    // Check if the key is currently held down (high bit set)
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

// ---------------- MENU STATE CHECKS -------------------

static bool IsMenuOpen(const char* menuName) {
    auto ui = RE::UI::GetSingleton();
    return ui && ui->IsMenuOpen(menuName);
}

static bool IsAnyRestrictedMenuOpen() {
    static const std::vector<const char*> restrictedMenus = {
        "InventoryMenu",
        "MagicMenu",
        "ContainerMenu",
        "BarterMenu",
        "GiftMenu",
        "MapMenu",
        "Journal Menu",
        "Lockpicking Menu",
        "Crafting Menu",
        "Console",
        "TweenMenu",
        "Book Menu",
        "StatsMenu",
        "Sleep/Wait Menu",
        "CustomMenu"
    };

    for (const auto& menu : restrictedMenus) {
        if (IsMenuOpen(menu)) {
            return true;
        }
    }
    return false;
}

// ---------------- INPUT HANDLER -------------------
class InputHandler : public RE::BSTEventSink<RE::InputEvent*> {
private:
    bool isStatsVisible = false;
    bool isBackSide = false;
    RE::ActorHandle lastTarget;

public:
    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* ev, RE::BSTEventSource<RE::InputEvent*>*) override {
        if (!ev || !*ev) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // ---------------------------------------------------------------
        // 1) UPDATE STATS ON ANY INPUT EVENT (MOUSE, KEYBOARD, CONTROLLER)
        // ---------------------------------------------------------------
        if (isStatsVisible) {
            // SAFETY CHECK: If a restricted menu is open, force close immediately
            if (IsAnyRestrictedMenuOpen()) {
                StatsCardMenu::UpdateActorStats(nullptr);
                isStatsVisible = false;
                lastTarget = RE::ActorHandle();
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::Actor* actor = GetLockedTargetFromTrueHUD();

            if (!actor) {
                StatsCardMenu::UpdateActorStats(nullptr);
                isStatsVisible = false;
                lastTarget = RE::ActorHandle();
            } else {
                auto currentHandle = actor->GetHandle();
                if (currentHandle != lastTarget) {
                    StatsCardMenu::UpdateActorStats(actor);
                    lastTarget = currentHandle;
                    // Play sound when switching to a different NPC
                    PlayUISound("UIMenuOK");
                }
            }
        }

        // ---------------------------------------------------------------
        // 2) Handle button events (hotkey toggle)
        // ---------------------------------------------------------------
        auto e = *ev;

        if (e->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) {
            // We already handled switching above (for mouse/controller)
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* btn = e->AsButtonEvent();
        if (!btn || !btn->IsDown()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto settings = Settings::GetSingleton();
        uint32_t keyCode = btn->GetIDCode();

        // -------------------------------
        // HOTKEY: Flip Card - Only if back card is enabled
        // -------------------------------
        if (keyCode == settings->flipHotkeyCode && settings->enableBackCard == 1) {
            // Check if restricted menus are open
            if (IsAnyRestrictedMenuOpen()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            // Check if required modifier key is pressed
            if (!IsModifierKeyPressed(settings->modifierKey)) {
                return RE::BSEventNotifyControl::kContinue;  // Modifier not pressed, ignore
            }

            auto ui = RE::UI::GetSingleton();
            if (!ui) {
                logger::warn("UI singleton not available");
                return RE::BSEventNotifyControl::kContinue;
            }

            auto menu = ui->GetMenu(StatsCardMenu::MENU_NAME);
            if (!menu || !menu->uiMovie) {
                logger::warn("Menu or uiMovie not available");
                return RE::BSEventNotifyControl::kContinue;
            }

            // Only flip if stats are visible
            if (isStatsVisible) {
                isBackSide = !isBackSide;

                menu->uiMovie->SetVariable("_root.StatsCard.FrontContainer._visible", RE::GFxValue(!isBackSide));
                menu->uiMovie->SetVariable("_root.StatsCard.StatsCardBack._visible", RE::GFxValue(isBackSide));

                // Play flip sound
                PlayUISound("UIMenuOK");
            } else {
                logger::warn("Stats not visible, cannot flip");
            }

            return RE::BSEventNotifyControl::kContinue;
        }

        // -------------------------------
        // HOTKEY: Toggle StatsCard UI (with modifier key support)
        // -------------------------------
        if (keyCode == settings->hotkeyCode) {
            // Check if restricted menus are open
            if (IsAnyRestrictedMenuOpen()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            // Check if required modifier key is pressed
            if (!IsModifierKeyPressed(settings->modifierKey)) {
                return RE::BSEventNotifyControl::kContinue;  // Modifier not pressed, ignore
            }

            // If visible: hide it
            if (isStatsVisible) {
                StatsCardMenu::UpdateActorStats(nullptr);
                isStatsVisible = false;
                lastTarget = RE::ActorHandle();
                return RE::BSEventNotifyControl::kContinue;
            }

            // Not visible: show stats for current target
            RE::Actor* target = GetLockedTargetFromTrueHUD();

            if (!target) {
                StatsCardMenu::UpdateActorStats(nullptr);
                isStatsVisible = false;
                lastTarget = RE::ActorHandle();
                return RE::BSEventNotifyControl::kContinue;
            }

            // Reset to front card if setting enabled (Option B)
            if (settings->resetCardOnOpen == 1) {
                isBackSide = false;
            }

            // If back card is disabled, always show front only
            if (settings->enableBackCard == 0) {
                isBackSide = false;
            }

            StatsCardMenu::Open();

            SKSE::GetTaskInterface()->AddTask([settings]() {
                auto ui = RE::UI::GetSingleton();
                if (!ui) return;

                auto menu = ui->GetMenu(StatsCardMenu::MENU_NAME);
                if (!menu || !menu->uiMovie) return;

                auto movie = menu->uiMovie;

                // Apply X
                movie->SetVariable("_root.StatsCard._x", RE::GFxValue((double)settings->posX));

                // Apply Y
                movie->SetVariable("_root.StatsCard._y", RE::GFxValue((double)settings->posY));

                // Apply Scale (percentage → scale factor)
                double scaleFactor = settings->scale / 100.0;
                movie->SetVariable("_root.StatsCard._xscale", RE::GFxValue(scaleFactor * 100));
                movie->SetVariable("_root.StatsCard._yscale", RE::GFxValue(scaleFactor * 100));
            });

            // Delay stats update by ONE FRAME (fixes initial load issue)
            SKSE::GetTaskInterface()->AddTask([handle = target->GetHandle()]() {
                if (auto actor = handle.get().get()) {
                    StatsCardMenu::UpdateActorStats(actor);
                }
            });

            // Apply the initial card state after opening
            SKSE::GetTaskInterface()->AddTask([settings, isBackSide = this->isBackSide]() {
                auto ui = RE::UI::GetSingleton();
                if (!ui) return;

                auto menu = ui->GetMenu(StatsCardMenu::MENU_NAME);
                if (!menu || !menu->uiMovie) return;

                // Only show back if enabled AND if isBackSide is true
                bool showBack = (settings->enableBackCard == 1) && isBackSide;

                menu->uiMovie->SetVariable("_root.StatsCard.FrontContainer._visible", RE::GFxValue(!showBack));
                menu->uiMovie->SetVariable("_root.StatsCard.StatsCardBack._visible", RE::GFxValue(showBack));
            });

            isStatsVisible = true;
            lastTarget = target->GetHandle();

            // Play open sound
            PlayUISound("UIMenuOK");

            return RE::BSEventNotifyControl::kContinue;
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

static InputHandler g_inputHandler;

// ---------------- SKSE PLUGIN LOAD ----------------
extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    // Logging
    wchar_t* documentsPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsPath);
    auto path = std::filesystem::path(documentsPath);
    CoTaskMemFree(documentsPath);

    path /= "My Games/Skyrim Special Edition/SKSE/StatsCard.log";

    std::filesystem::create_directories(path.parent_path());

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
    auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
    spdlog::set_default_logger(std::move(log));

    logger::info("StatsCard Plugin Loading…");

    Settings::GetSingleton()->Load();

    // NEW: Load localization after settings
    auto loc = LocalizationManager::GetSingleton();
    loc->LoadLanguage(Settings::GetSingleton()->language);

    StatsCardMenu::Register();

    if (auto messaging = SKSE::GetMessagingInterface()) {
        messaging->RegisterListener([](SKSE::MessagingInterface::Message* msg) {
            switch (msg->type) {
                case SKSE::MessagingInterface::kPostLoad:
                    InitTDMAPI();
                    break;

                case SKSE::MessagingInterface::kInputLoaded:
                    if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
                        input->AddEventSink(&g_inputHandler);
                        logger::info("InputHandler registered");
                    }
                    break;

                // After load screen / save load, re-register input handler
                case SKSE::MessagingInterface::kPostLoadGame:
                    if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
                        input->AddEventSink(&g_inputHandler);
                        logger::info("InputHandler re-registered [kPostLoadGame]");
                    }
                    break;

                case SKSE::MessagingInterface::kDataLoaded:
                    logger::info("Data loaded");
                    break;

                default:
                    break;
            }
        });
    }

    logger::info("StatsCard Plugin Loaded");
    return true;
}
