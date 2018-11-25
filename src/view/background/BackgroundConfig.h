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
#include <string>
using std::map;
using std::string;

class BackgroundConfig
{
public:
	BackgroundConfig(string config);
	virtual ~BackgroundConfig();

public:
	bool loadValue(string key, string& value);
	bool loadValue(string key, int& value);
	bool loadValue(string key, double& value);
	bool loadValueHex(string key, int& value);

private:
	XOJ_TYPE_ATTRIB;

	map<string, string> data;
};
