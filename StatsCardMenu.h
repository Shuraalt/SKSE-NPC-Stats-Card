#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

class StatsCardMenu : public RE::IMenu {
public:
    static inline constexpr const char* MENU_NAME = "StatsCardMenu";

    StatsCardMenu();
    ~StatsCardMenu() override = default;

    static void Register();
    static void Open();
    static void Close();
    static bool IsOpen();

    // actor == nullptr → hide card; otherwise show + update stats
    static void UpdateActorStats(RE::Actor* actor);

    void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override;
};
