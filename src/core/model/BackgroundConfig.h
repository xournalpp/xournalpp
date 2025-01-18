/*
 * Xournal++
 *
 * Configuration data for generated backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdint>    // for uint32_t
#include <map>        // for map
#include <string>     // for string


// This class corresponds to the config line of entries in pagetemplate.ini
class BackgroundConfig {
public:
    BackgroundConfig(const std::string& config);
    virtual ~BackgroundConfig();

public:
    bool loadValue(const std::string& key, std::string& value) const;
    bool loadValue(const std::string& key, int& value) const;
    bool loadValue(const std::string& key, double& value) const;
    bool loadValueHex(const std::string& key, uint32_t& value) const;

private:
    std::map<std::string, std::string> data;
};

namespace background_config_strings {
// Those strings are used in the pagetemplate.ini configuration file
constexpr static char CFG_FOREGROUND_COLOR_1[] = "f1";
constexpr static char CFG_ALT_FOREGROUND_COLOR_1[] = "af1";
constexpr static char CFG_FOREGROUND_COLOR_2[] = "f2";
constexpr static char CFG_ALT_FOREGROUND_COLOR_2[] = "af2";
constexpr static char CFG_LINE_WIDTH[] = "lw";
constexpr static char CFG_MARGIN[] = "m1";
constexpr static char CFG_ROUND_MARGIN[] = "rm";
constexpr static char CFG_RASTER[] = "r1";
}  // namespace background_config_strings
