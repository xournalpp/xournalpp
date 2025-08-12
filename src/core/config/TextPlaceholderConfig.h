// TextPlaceholderConfig.h
#pragma once
#include <string>
#include <unordered_map>

class TextPlaceholderConfig {
public:
    TextPlaceholderConfig(const std::string& iniPath);
    std::string getValue(const std::string& name) const;
    void setValue(const std::string& name, const std::string& value);
    void save() const;
    void reload();
    const std::unordered_map<std::string, std::string>& getPlaceholders() const { return placeholders; }
private:
    std::string iniPath;
    std::unordered_map<std::string, std::string> placeholders;
    void load();
};
