#pragma once

#include <string>
#include <unordered_map>

class LocalizationManager {
public:
    static LocalizationManager* GetSingleton() {
        static LocalizationManager singleton;
        return &singleton;
    }

    // Load language file based on settings
    void LoadLanguage(const std::string& languageCode);

    // Get localized string by key
    std::string GetLabel(const std::string& key) const;

private:
    LocalizationManager() = default;
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    // Load specific language file
    bool LoadLanguageFile(const std::wstring& filename, std::unordered_map<std::string, std::string>& target);

    std::unordered_map<std::string, std::string> labels;          // Current language
    std::unordered_map<std::string, std::string> englishFallback; // English fallback
};