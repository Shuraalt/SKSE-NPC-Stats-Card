// Block in all major menus except dialogue
if (UI::IsMenuOpen("InventoryMenu") || 
    UI::IsMenuOpen("MagicMenu") || 
    UI::IsMenuOpen("ContainerMenu") ||
    UI::IsMenuOpen("MapMenu") ||
    UI::IsMenuOpen("JournalMenu") ||
    UI::IsMenuOpen("BookMenu") ||
    UI::IsMenuOpen("CraftingMenu") ||
    UI::IsMenuOpen("SmithingMenu") ||
    UI::IsMenuOpen("EnchantingMenu") ||
    UI::IsMenuOpen("AlchemyMenu") ||
    UI::IsMenuOpen("LockpickingMenu") ||
    UI::IsMenuOpen("StatsMenu") ||
    UI::IsMenuOpen("SleepWaitMenu") ||
    UI::IsMenuOpen("CustomMenu")) { // SkyUI MCM
    return;
}