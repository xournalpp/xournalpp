/*
 * Xournal++
 *
 * Xournal main entry, commandline parser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <config.h>
#include <XournalType.h>

#include <string>
using std::string;

class GladeSearchpath;

class XournalMain
{
public:
	XournalMain();
	virtual ~XournalMain();

public:
	int run(int argc, char* argv[]);

private:
	void initLocalisation();

	void checkForErrorlog();
	void checkForEmergencySave();

	int exportPdf(const char* input, const char* output);
	void initSettingsPath();
	void initResourcePath(GladeSearchpath* gladePath);
	string findResourcePath(string searchFile);

private:
	XOJ_TYPE_ATTRIB;

};
