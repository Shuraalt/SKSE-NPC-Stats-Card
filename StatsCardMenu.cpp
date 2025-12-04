#include "StatsCardMenu.h"

#include <algorithm>
#include <set>
#include "LocalizationManager.h"

using namespace RE;

// Factory function for UI::Register (no lambdas in your CLib)
static RE::IMenu* CreateStatsCardMenu() { return new StatsCardMenu(); }

void StatsCardMenu::Register() {
    auto ui = RE::UI::GetSingleton();
    if (ui) {
        ui->Register(MENU_NAME, CreateStatsCardMenu);
        logger::info("StatsCardMenu registered");
    } else {
        logger::error("Failed to register StatsCardMenu - UI singleton not available");
    }
}

StatsCardMenu::StatsCardMenu() {
    using Flag = RE::IMenu::Flag;
    menuFlags.set(Flag::kAlwaysOpen);
    menuFlags.set(Flag::kAllowSaving);
    depthPriority = 0;

    auto scaleform = RE::BSScaleformManager::GetSingleton();
    if (!scaleform) {
        logger::error("ScaleformManager not found");
        return;
    }

    scaleform->LoadMovie(this, this->uiMovie, "StatsCard", RE::BSScaleformManager::ScaleModeType::kShowAll, 0.0f);

    if (uiMovie) {
        uiMovie->Advance(0.0f);
        logger::info("StatsCard menu initialized");
    } else {
        logger::error("Failed to load StatsCard.swf");
    }
}

void StatsCardMenu::AdvanceMovie(float a_interval, std::uint32_t) {
    if (uiMovie) {
        uiMovie->Advance(a_interval);
    }
}

void StatsCardMenu::Open() {
    if (auto queue = RE::UIMessageQueue::GetSingleton()) {
        queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
}

void StatsCardMenu::Close() {
    if (auto queue = RE::UIMessageQueue::GetSingleton()) {
        queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

bool StatsCardMenu::IsOpen() {
    auto ui = RE::UI::GetSingleton();
    return ui && ui->IsMenuOpen(MENU_NAME);
}

// Helper macro for easy access
#define LOC(key) LocalizationManager::GetSingleton()->GetLabel(key)

// ========================================
// UPDATED HELPER FUNCTIONS
// ========================================

std::string GetNPCPerks(RE::Actor* actor) {
    if (!actor) return LOC("NoPerks");

    auto actorBase = actor->GetActorBase();
    if (!actorBase) return LOC("NoPerks");

    if (!actorBase->perks || actorBase->perkCount == 0) {
        return LOC("NoPerks");
    }

    std::string perks;
    std::set<std::string> perkSet;

    std::uint32_t maxPerks = (std::min)(actorBase->perkCount, static_cast<std::uint32_t>(15));

    for (std::uint32_t i = 0; i < maxPerks; ++i) {
        auto& perkData = actorBase->perks[i];
        if (perkData.perk) {
            const char* perkName = perkData.perk->GetFullName();
            if (perkName && std::strlen(perkName) > 0) {
                perkSet.insert(perkName);
            }
        }
    }

    for (const auto& perkName : perkSet) {
        if (!perks.empty()) perks += ", ";
        perks += perkName;
    }

    if (actorBase->perkCount > 15) {
        perks += "...";
    }

    return perks.empty() ? LOC("NoPerks") : perks;
}

std::string GetNPCSpells(RE::Actor* actor) {
    if (!actor) return LOC("NoSpells");

    auto actorBase = actor->GetActorBase();
    if (!actorBase) return LOC("NoSpells");

    if (!actorBase->actorEffects || !actorBase->actorEffects->spells || actorBase->actorEffects->numSpells == 0) {
        return LOC("NoSpells");
    }

    std::string spells;
    std::set<std::string> spellSet;

    std::uint32_t maxSpells = (std::min)(actorBase->actorEffects->numSpells, static_cast<std::uint32_t>(10));

    for (std::uint32_t i = 0; i < maxSpells; ++i) {
        auto spell = actorBase->actorEffects->spells[i];
        if (spell) {
            const char* spellName = spell->GetFullName();
            if (spellName && std::strlen(spellName) > 0) {
                spellSet.insert(spellName);
            }
        }
    }

    for (const auto& spellName : spellSet) {
        if (!spells.empty()) spells += ", ";
        spells += spellName;
    }

    if (actorBase->actorEffects->numSpells > 10) {
        spells += "...";
    }

    return spells.empty() ? LOC("NoSpells") : spells;
}

std::string GetNPCAIData(RE::Actor* actor) {
    if (!actor) return LOC("Unknown");

    auto actorBase = actor->GetActorBase();
    if (!actorBase) return LOC("Unknown");

    std::string aiInfo;

    // Aggression
    auto aggression = static_cast<std::uint8_t>(actorBase->GetAggressionLevel());
    switch (aggression) {
        case 0:
            aiInfo += LOC("AI_Aggression.Unaggressive");
            break;
        case 1:
            aiInfo += LOC("AI_Aggression.Aggressive");
            break;
        case 2:
            aiInfo += LOC("AI_Aggression.VeryAggressive");
            break;
        case 3:
            aiInfo += LOC("AI_Aggression.Frenzied");
            break;
        default:
            aiInfo += LOC("Unknown");
            break;
    }

    // Confidence
    aiInfo += " - ";
    auto confidence = static_cast<std::uint8_t>(actorBase->GetConfidenceLevel());
    switch (confidence) {
        case 0:
            aiInfo += LOC("AI_Confidence.Cowardly");
            break;
        case 1:
            aiInfo += LOC("AI_Confidence.Cautious");
            break;
        case 2:
            aiInfo += LOC("AI_Confidence.Average");
            break;
        case 3:
            aiInfo += LOC("AI_Confidence.Brave");
            break;
        case 4:
            aiInfo += LOC("AI_Confidence.Foolhardy");
            break;
        default:
            aiInfo += LOC("Unknown");
            break;
    }

    // Assistance
    aiInfo += " - ";
    auto assistance = static_cast<std::uint8_t>(actorBase->GetAssistanceLevel());
    switch (assistance) {
        case 0:
            aiInfo += LOC("AI_Assistance.None");
            break;
        case 1:
            aiInfo += LOC("AI_Assistance.Allies");
            break;
        case 2:
            aiInfo += LOC("AI_Assistance.FriendsAndAllies");
            break;
        default:
            aiInfo += LOC("Unknown");
            break;
    }

    // Morality
    aiInfo += " - ";
    auto morality = static_cast<std::uint8_t>(actorBase->GetMoralityLevel());
    switch (morality) {
        case 0:
            aiInfo += LOC("AI_Morality.AnyCrime");
            break;
        case 1:
            aiInfo += LOC("AI_Morality.ViolenceOnly");
            break;
        case 2:
            aiInfo += LOC("AI_Morality.PropertyOnly");
            break;
        case 3:
            aiInfo += LOC("AI_Morality.NoCrime");
            break;
        default:
            aiInfo += LOC("Unknown");
            break;
    }

    return aiInfo;
}

std::string GetNPCRelationship(RE::Actor* actor) {
    if (!actor) return LOC("None");

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return LOC("None");

    std::int32_t relationshipRank = 0;
    bool isHostile = actor->IsHostileToActor(player);

    if (isHostile) {
        relationshipRank = -2;
    } else {
        bool foundFriendlyFaction = false;

        actor->VisitFactions([&](RE::TESFaction* faction, std::int8_t) -> bool {
            if (faction) {
                player->VisitFactions([&](RE::TESFaction* playerFaction, std::int8_t) -> bool {
                    if (playerFaction == faction) {
                        foundFriendlyFaction = true;
                        relationshipRank = 1;
                        return false;
                    }
                    return true;
                });

                if (foundFriendlyFaction) {
                    return false;
                }
            }
            return true;
        });
    }

    // Return localized relationship string
    if (relationshipRank >= 4) {
        return LOC("Relationship.Lover");
    } else if (relationshipRank == 3) {
        return LOC("Relationship.Ally");
    } else if (relationshipRank == 2) {
        return LOC("Relationship.Confidant");
    } else if (relationshipRank == 1) {
        return LOC("Relationship.Friend");
    } else if (relationshipRank == 0) {
        return LOC("Relationship.Acquaintance");
    } else if (relationshipRank == -1) {
        return LOC("Relationship.Rival");
    } else if (relationshipRank == -2) {
        return LOC("Relationship.Foe");
    } else if (relationshipRank == -3) {
        return LOC("Relationship.Enemy");
    } else if (relationshipRank <= -4) {
        return LOC("Relationship.Archnemesis");
    }

    return LOC("Relationship.Acquaintance");
}

// ========================================
// TOP SKILLS HELPER (Combat, Armor, Magic)
// ========================================

struct SkillData {
    std::string name;
    float value;
};

struct TopSkillsResult {
    SkillData combat[2];
    SkillData armor[2];
    SkillData magic[2];
};

TopSkillsResult GetNPCTopSkills(RE::Actor* actor) {
    TopSkillsResult result;

    // Initialize with empty/zero values
    for (int i = 0; i < 2; ++i) {
        result.combat[i] = {"", 0.0f};
        result.armor[i] = {"", 0.0f};
        result.magic[i] = {"", 0.0f};
    }

    if (!actor) return result;

    auto avo = actor->AsActorValueOwner();
    if (!avo) return result;

    auto R = [&](RE::ActorValue av) -> float { return avo->GetActorValue(av); };

    // COMBAT SKILLS (3 skills) - Using localized names
    std::vector<SkillData> combatSkills = {
        {LOC("Skills.OneHanded"), R(RE::ActorValue::kOneHanded)},
        {LOC("Skills.TwoHanded"), R(RE::ActorValue::kTwoHanded)},
        {LOC("Skills.Archery"), R(RE::ActorValue::kArchery)}
    };

    // ARMOR SKILLS (3 skills) - Using localized names
    std::vector<SkillData> armorSkills = {
        {LOC("Skills.HeavyArmor"), R(RE::ActorValue::kHeavyArmor)},
        {LOC("Skills.LightArmor"), R(RE::ActorValue::kLightArmor)},
        {LOC("Skills.Block"), R(RE::ActorValue::kBlock)}
    };

    // MAGIC SKILLS (5 skills) - Using localized names
    std::vector<SkillData> magicSkills = {
        {LOC("Skills.Destruction"), R(RE::ActorValue::kDestruction)},
        {LOC("Skills.Conjuration"), R(RE::ActorValue::kConjuration)},
        {LOC("Skills.Restoration"), R(RE::ActorValue::kRestoration)},
        {LOC("Skills.Illusion"), R(RE::ActorValue::kIllusion)},
        {LOC("Skills.Alteration"), R(RE::ActorValue::kAlteration)}
    };

    // Sort each category by value (descending)
    auto sortByValue = [](const SkillData& a, const SkillData& b) { return a.value > b.value; };

    std::sort(combatSkills.begin(), combatSkills.end(), sortByValue);
    std::sort(armorSkills.begin(), armorSkills.end(), sortByValue);
    std::sort(magicSkills.begin(), magicSkills.end(), sortByValue);

    // Get top 2 from each category
    for (int i = 0; i < 2; ++i) {
        result.combat[i] = combatSkills[i];
        result.armor[i] = armorSkills[i];
        result.magic[i] = magicSkills[i];
    }

    return result;
}

// ========================================
// MAIN UPDATE FUNCTION (EXTENDED)
// ========================================

void StatsCardMenu::UpdateActorStats(RE::Actor* actor) {
    auto ui = RE::UI::GetSingleton();
    if (!ui) {
        logger::warn("UpdateActorStats: UI singleton not available");
        return;
    }

    auto menu = ui->GetMenu(MENU_NAME);
    if (!menu) {
        logger::info("StatsCardMenu not open, sending Open()");
        StatsCardMenu::Open();
        return;
    }

    if (!menu->uiMovie) {
        logger::warn("UpdateActorStats: uiMovie not available, reloading StatsCard.swf");

        auto scaleform = RE::BSScaleformManager::GetSingleton();
        if (!scaleform) {
            logger::error("ScaleformManager not found while trying to reload uiMovie");
            return;
        }

        scaleform->LoadMovie(menu.get(), menu->uiMovie, "StatsCard", RE::BSScaleformManager::ScaleModeType::kShowAll,
                             0.0f);

        if (menu->uiMovie) {
            menu->uiMovie->Advance(0.0f);
            logger::info("StatsCard uiMovie reloaded successfully");
        } else {
            logger::error("StatsCard uiMovie reload failed");
            return;
        }
    }

    auto movie = menu->uiMovie;

    if (!actor) {
        movie->Invoke("_root.StatsCard.fadeOut", nullptr, nullptr, 0);
        return;
    }

    auto avo = actor->AsActorValueOwner();
    if (!avo) {
        logger::warn("Actor has no ActorValueOwner");
        return;
    }

    auto R = [&](RE::ActorValue av) -> float { return avo->GetActorValue(av); };

    // Basic stats
    const char* name = actor->GetName();
    if (!name) name = "";

    double level = actor->GetLevel();
    double hp = R(RE::ActorValue::kHealth);
    double hpMax = avo->GetPermanentActorValue(RE::ActorValue::kHealth);
    double mag = R(RE::ActorValue::kMagicka);
    double magMax = avo->GetPermanentActorValue(RE::ActorValue::kMagicka);
    double stam = R(RE::ActorValue::kStamina);
    double stamMax = avo->GetPermanentActorValue(RE::ActorValue::kStamina);

    // Resistances
    float fireRes = R(RE::ActorValue::kResistFire);
    float frostRes = R(RE::ActorValue::kResistFrost);
    float shockRes = R(RE::ActorValue::kResistShock);
    float magicRes = R(RE::ActorValue::kResistMagic);
    float diseaseRes = R(RE::ActorValue::kResistDisease);
    float poisonRes = R(RE::ActorValue::kPoisonResist);
    float armor = R(RE::ActorValue::kDamageResist);

    // Race name - with fallback
    std::string raceName = LOC("Unknown");
    if (auto race = actor->GetRace()) {
        if (race->GetFullName()) {
            raceName = race->GetFullName();
        }
    }

    // Class name - with fallback
    std::string className = LOC("Unknown");
    auto actorBase = actor->GetActorBase();
    if (actorBase && actorBase->GetFormType() == RE::FormType::NPC) {
        auto npc = skyrim_cast<RE::TESNPC*>(actorBase);
        if (npc && npc->npcClass) {
            const char* classFullName = npc->npcClass->GetFullName();
            if (classFullName && std::strlen(classFullName) > 0) {
                className = classFullName;
            }
        }
    }

    // Factions - with fallback
    std::string factions;
    actor->VisitFactions([&](RE::TESFaction* faction, std::int8_t) -> bool {
        if (faction && faction->GetFullName()) {
            if (!factions.empty()) {
                factions += ", ";
            }
            factions += std::string(faction->GetFullName());
        }
        return true;
    });
    if (factions.empty()) {
        factions = LOC("None");
    }

    // Quest involvement
    std::string quests;
    std::set<RE::TESQuest*> foundQuests;

    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler) {
        // Reuse actorBase from above (already declared at line 456)
        auto actorFormID = actor->GetFormID();

        for (auto& quest : dataHandler->GetFormArray<RE::TESQuest>()) {
            if (!quest) continue;

            if (quest->data.questType == RE::QUEST_DATA::Type::kNone) {
                continue;
            }

            bool isInvolved = false;

            for (auto& alias : quest->aliases) {
                if (!alias) continue;

                if (auto refAlias = skyrim_cast<RE::BGSRefAlias*>(alias)) {
                    auto aliasRef = refAlias->GetReference();

                    if (aliasRef && (aliasRef == actor || aliasRef->GetFormID() == actorFormID)) {
                        isInvolved = true;
                        break;
                    }

                    if (refAlias->fillType == RE::BGSBaseAlias::FILL_TYPE::kForced) {
                        auto forcedRef = refAlias->GetReference();
                        if (forcedRef && forcedRef->GetFormID() == actorFormID) {
                            isInvolved = true;
                            break;
                        }
                    }

                    if (refAlias->fillType == RE::BGSBaseAlias::FILL_TYPE::kUniqueActor) {
                        if (aliasRef && aliasRef->GetBaseObject() == actorBase) {
                            isInvolved = true;
                            break;
                        }
                    }
                }
            }

            if (isInvolved) {
                foundQuests.insert(quest);
            }
        }
    }

    for (auto quest : foundQuests) {
        if (!quests.empty()) {
            quests += ", ";
        }

        const char* questName = quest->GetFullName();
        if (questName && std::strlen(questName) > 0) {
            quests += questName;
        } else {
            const char* editorID = quest->GetFormEditorID();
            if (editorID) {
                quests += editorID;
            } else {
                quests += std::format("Quest_{:08X}", quest->GetFormID());
            }
        }
    }
    if (quests.empty()) {
        quests = LOC("NoQuests");
    }

    // ⭐ NEW: Collect extended NPC data
    std::string perks = GetNPCPerks(actor);
    std::string spells = GetNPCSpells(actor);
    std::string aiData = GetNPCAIData(actor);
    std::string relationship = GetNPCRelationship(actor);

    // ⭐ NEW: Get top skills (Combat, Armor, Magic - 2 from each)
    TopSkillsResult topSkills = GetNPCTopSkills(actor);

    // Format skills as strings for display
    std::string combat1 = std::format("{}: {:.0f}", topSkills.combat[0].name, topSkills.combat[0].value);
    std::string combat2 = std::format("{}: {:.0f}", topSkills.combat[1].name, topSkills.combat[1].value);
    std::string armor1 = std::format("{}: {:.0f}", topSkills.armor[0].name, topSkills.armor[0].value);
    std::string armor2 = std::format("{}: {:.0f}", topSkills.armor[1].name, topSkills.armor[1].value);
    std::string magic1 = std::format("{}: {:.0f}", topSkills.magic[0].name, topSkills.magic[0].value);
    std::string magic2 = std::format("{}: {:.0f}", topSkills.magic[1].name, topSkills.magic[1].value);

    // Pass all data to ActionScript (23 base data + 6 skills + 18 labels = 47 parameters)
    RE::GFxValue args[47];

    // Data parameters (0-22)
    args[0] = RE::GFxValue(name);
    args[1] = RE::GFxValue(level);
    args[2] = RE::GFxValue(hp);
    args[3] = RE::GFxValue(hpMax);
    args[4] = RE::GFxValue(mag);
    args[5] = RE::GFxValue(magMax);
    args[6] = RE::GFxValue(stam);
    args[7] = RE::GFxValue(stamMax);
    args[8] = RE::GFxValue(fireRes);
    args[9] = RE::GFxValue(frostRes);
    args[10] = RE::GFxValue(shockRes);
    args[11] = RE::GFxValue(magicRes);
    args[12] = RE::GFxValue(diseaseRes);
    args[13] = RE::GFxValue(poisonRes);
    args[14] = RE::GFxValue(armor);
    args[15] = RE::GFxValue(raceName.c_str());
    args[16] = RE::GFxValue(className.c_str());
    args[17] = RE::GFxValue(factions.c_str());
    args[18] = RE::GFxValue(quests.c_str());
    args[19] = RE::GFxValue(perks.c_str());
    args[20] = RE::GFxValue(spells.c_str());
    args[21] = RE::GFxValue(aiData.c_str());
    args[22] = RE::GFxValue(relationship.c_str());

    // Skill parameters (23-28) - All 6 skills
    args[23] = RE::GFxValue(combat1.c_str());
    args[24] = RE::GFxValue(combat2.c_str());
    args[25] = RE::GFxValue(armor1.c_str());
    args[26] = RE::GFxValue(armor2.c_str());
    args[27] = RE::GFxValue(magic1.c_str());
    args[28] = RE::GFxValue(magic2.c_str());

    // Store labels in local variables so they stay alive
    std::string lLevel = LOC("Level");
    std::string lHP = LOC("HP");
    std::string lMP = LOC("MP");
    std::string lSP = LOC("SP");
    std::string lFire = LOC("Fire");
    std::string lFrost = LOC("Frost");
    std::string lShock = LOC("Shock");
    std::string lMagic = LOC("Magic");
    std::string lDisease = LOC("Disease");
    std::string lPoison = LOC("Poison");
    std::string lArmor = LOC("Armor");
    std::string lRace = LOC("Race");
    std::string lClass = LOC("Class");
    std::string lQuests = LOC("Quests");
    std::string lPerks = LOC("Perks");
    std::string lSpells = LOC("Spells");
    std::string lAI = LOC("AI");
    std::string lRelation = LOC("Relation");

    // Label parameters (29-46)
    args[29] = RE::GFxValue(lLevel.c_str());
    args[30] = RE::GFxValue(lHP.c_str());
    args[31] = RE::GFxValue(lMP.c_str());
    args[32] = RE::GFxValue(lSP.c_str());
    args[33] = RE::GFxValue(lFire.c_str());
    args[34] = RE::GFxValue(lFrost.c_str());
    args[35] = RE::GFxValue(lShock.c_str());
    args[36] = RE::GFxValue(lMagic.c_str());
    args[37] = RE::GFxValue(lDisease.c_str());
    args[38] = RE::GFxValue(lPoison.c_str());
    args[39] = RE::GFxValue(lArmor.c_str());
    args[40] = RE::GFxValue(lRace.c_str());
    args[41] = RE::GFxValue(lClass.c_str());
    args[42] = RE::GFxValue(lQuests.c_str());
    args[43] = RE::GFxValue(lPerks.c_str());
    args[44] = RE::GFxValue(lSpells.c_str());
    args[45] = RE::GFxValue(lAI.c_str());
    args[46] = RE::GFxValue(lRelation.c_str());

    movie->Invoke("_root.StatsCard.SetStats", nullptr, args, 47);
    movie->SetVariable("_root.StatsCard._visible", RE::GFxValue(true));
}