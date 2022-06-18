#include "BackgroundConfig.h"

#include <cstddef>  // for size_t
#include <sstream>  // for istringstream, basic_ios::imbue, basi...
#include <utility>  // for pair
#include <vector>   // for vector

#include "util/StringUtils.h"   // for StringUtils
#include "util/serdesstream.h"  // for serdes_stream

using std::string;

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

auto BackgroundConfig::loadValue(const string& key, string& value) const -> bool {
    auto it = data.find(key);
    if (it != this->data.end()) {
        value = it->second;
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValue(const string& key, int& value) const -> bool {
    string str;
    if (loadValue(key, str)) {
        auto valueStream = serdes_stream<std::istringstream>(str);
        valueStream >> value;
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValue(const string& key, double& value) const -> bool {
    string str;
    if (loadValue(key, str)) {
        auto valueStream = serdes_stream<std::istringstream>(str);
        valueStream >> value;
        return true;
    }

    return false;
}

auto BackgroundConfig::loadValueHex(const string& key, uint32_t& value) const -> bool {
    string str;
    if (loadValue(key, str)) {
        auto valueStream = serdes_stream<std::istringstream>(str);
        valueStream >> std::hex >> value;
        return true;
    }

    return false;
}
