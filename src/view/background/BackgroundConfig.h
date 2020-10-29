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

#include "XournalType.h"
using std::map;

class BackgroundConfig {
public:
    BackgroundConfig(const string& config);
    virtual ~BackgroundConfig();

public:
    bool loadValue(const string& key, string& value) const;
    bool loadValue(const string& key, int& value) const;
    bool loadValue(const string& key, double& value) const;
    bool loadValueHex(const string& key, int& value) const;
    bool loadValueHex(const string& key, uint32_t& value) const;

private:
    map<string, string> data;
};
