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

#include <XournalType.h>

#include <map>
using std::map;

class BackgroundConfig
{
public:
	BackgroundConfig(const string& config);
	virtual ~BackgroundConfig();

public:
	bool loadValue(const string& key, string& value);
	bool loadValue(const string& key, int& value);
	bool loadValue(const string& key, double& value);
	bool loadValueHex(const string& key, int& value);

private:
	map<string, string> data;
};
