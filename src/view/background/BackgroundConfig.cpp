#include "BackgroundConfig.h"

#include <sstream>
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

bool BackgroundConfig::setValue(const string& key, string value) {
    this->data[key] = value;

    return true;
}

bool BackgroundConfig::setValueHex(const string& key, uint32_t value) {
    char output[6];

    sprintf(output, "%X", value);

    this->setValue(key, output);
    return true;
}

string BackgroundConfig::toString() {
    map<string, string>::iterator it;

    // string output = (char *) malloc(64);

    string output;

    for (it = this->data.begin(); it != this->data.end(); it++) {
        if (it != this->data.begin())
            output += ',';
        output += it->first + '=' + it->second;
    }

    return output;
}
