/*
 * Xournal++
 *
 * Selects and configures the right Background Painter Class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>
#include <string>


class BackgroundConfig {
public:
    BackgroundConfig(const std::string& config);
    virtual ~BackgroundConfig();

public:
    bool loadValue(const std::string& key, std::string& value) const;
    bool loadValue(const std::string& key, int& value) const;
    bool loadValue(const std::string& key, double& value) const;
    bool loadValueHex(const std::string& key, int& value) const;
    bool loadValueHex(const std::string& key, uint32_t& value) const;

private:
    std::map<std::string, std::string> data;
};
