#include "LocalizationManager.h"

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>

// Put this at the top of your file or in a utility header
inline std::string WStrToStr(const std::wstring& wstr) {
#pragma warning(push)
#pragma warning(disable : 4244)
    return std::string(wstr.begin(), wstr.end());
#pragma warning(pop)
}

void LocalizationManager::LoadLanguage(const std::string& languageCode) {
    // Clear existing labels
    labels.clear();
    englishFallback.clear();

    // Build file paths
    std::wstring basePath = L"Data/SKSE/Plugins/";
    std::wstring langFile = basePath + std::wstring(languageCode.begin(), languageCode.end()) + L".ini";
    std::wstring enFile = basePath + L"StatsCard_en.ini";

  //  logger::info("Loading language file: {}", std::string(langFile.begin(), langFile.end()));
    logger::info("Loading language file: {}", WStrToStr(langFile));

    // Always load English as fallback first
    if (!LoadLanguageFile(enFile, englishFallback)) {
        logger::warn("Could not load English fallback file: StatsCard_en.ini");
    } else {
        logger::info("English fallback loaded successfully ({} keys)", englishFallback.size());
    }

    // Load selected language (if not English)
    if (languageCode != "StatsCard_en") {
        if (LoadLanguageFile(langFile, labels)) {
            logger::info("Language file loaded successfully ({} keys)", labels.size());
        } else {
            logger::warn("Could not load language file, using English");
            labels = englishFallback; // Use English if selected language fails
        }
    } else {
        // If English is selected, use it directly
        labels = englishFallback;
    }
}

bool LocalizationManager::LoadLanguageFile(const std::wstring& filename, 
                                           std::unordered_map<std::string, std::string>& target) {
    // Check if file exists
    if (!std::filesystem::exists(filename)) {
        return false;
    }

    // Open file with UTF-8 encoding support
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read entire file
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Skip UTF-8 BOM if present
    if (content.size() >= 3 && 
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    // Parse INI file
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;

    while (std::getline(stream, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Check for section header [Section]
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);

            // Trim key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Store with section prefix (e.g., "Labels.HP")
            if (!currentSection.empty()) {
                target[currentSection + "." + key] = value;
            }
            // Also store without prefix for backwards compatibility
            target[key] = value;
        }
    }

    return !target.empty();
}

std::string LocalizationManager::GetLabel(const std::string& key) const {
    // 1. Try selected language
    auto it = labels.find(key);
    if (it != labels.end()) {
        return it->second;
    }

    // 2. Try English fallback
    it = englishFallback.find(key);
    if (it != englishFallback.end()) {
        return it->second;
    }

    // 3. Return key as last resort (hardcoded fallback)
    logger::warn("Missing translation for key: {}", key);
    return key;
}