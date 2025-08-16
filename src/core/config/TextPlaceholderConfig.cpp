// TextPlaceholderConfig.cpp
#include "TextPlaceholderConfig.h"
#include <fstream>
#include <sstream>
#include <algorithm>

TextPlaceholderConfig::TextPlaceholderConfig(const std::string& iniPath): iniPath(iniPath) {
    load();
}

void TextPlaceholderConfig::load() {
    placeholders.clear();
    std::ifstream file(iniPath);
    std::string line, currentSection;
    while (std::getline(file, line)) {
        // Only trim leading/trailing whitespace, not all whitespace
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty() || line[0] == '#') continue;
        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        } else if (line.find("value=") == 0 && !currentSection.empty()) {
            std::string value = line.substr(6);
            // Unescape \n to real newlines
            size_t pos = 0;
            while ((pos = value.find("\\n", pos)) != std::string::npos) {
                value.replace(pos, 2, "\n");
                pos += 1;
            }
            placeholders[currentSection] = value;
        }
    }
}

std::string TextPlaceholderConfig::getValue(const std::string& name) const {
    auto it = placeholders.find(name);
        if (it != placeholders.end()) {
            std::string value = it->second;
            // Unescape \n to real newlines
            size_t pos = 0;
            while ((pos = value.find("\\n", pos)) != std::string::npos) {
                value.replace(pos, 2, "\n");
                pos += 1;
            }
            return value;
        }
    return "none";
}

void TextPlaceholderConfig::setValue(const std::string& name, const std::string& value) {
    // Escape real newlines to \n for config storage
    std::string escaped = value;
    size_t pos = 0;
    while ((pos = escaped.find("\n", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\n");
        pos += 2;
    }
    placeholders[name] = escaped;
}

void TextPlaceholderConfig::save() const {
    std::ofstream file(iniPath);
    for (const auto& kv : placeholders) {
        file << "[" << kv.first << "]\nvalue=" << kv.second << "\n\n";
    }
}

void TextPlaceholderConfig::reload() {
    load();
}
