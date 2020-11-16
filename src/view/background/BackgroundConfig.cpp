#include "BackgroundConfig.h"

#include <utility>

#include "StringUtils.h"

BackgroundConfig::BackgroundConfig(const string& config) {
    for (const string& s: StringUtils::split(config, ',')) {
        size_t dotPos = s.find_last_of('=');
        if (dotPos != string::npos) {
            string key = s.substr(0, dotPos);
            string value = s.substr(dotPos + 1);
            data[key] = value;
        }
    }
}

BackgroundConfig::~BackgroundConfig() = default;

auto BackgroundConfig::loadValue(const string& key, string& value) -> bool {
    auto it = data.find(key);
    if (it != this->data.end()) {
        value = it->second;
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValue(const string& key, int& value) -> bool {
    string str;
    if (loadValue(key, str)) {
        value = std::stoul(str, nullptr, 10);
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValue(const string& key, double& value) -> bool {
    string str;
    if (loadValue(key, str)) {
        value = std::stoul(str, nullptr, 10);
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValueHex(const string& key, int& value) -> bool {
    string str;
    if (loadValue(key, str)) {
        value = std::stoul(str, nullptr, 16);
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValueHex(const string& key, uint32_t& value) -> bool {
    string str;
    if (loadValue(key, str)) {
        value = std::stoul(str, nullptr, 16);
        return true;
    }

    return false;
}
