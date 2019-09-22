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

class GladeSearchpath;
class Control;

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
	void checkForEmergencySave(Control* control);

	int exportPdf(const char* input, const char* output);
	int exportImg(const char* input, const char* output);

	void initSettingsPath();
	void initResourcePath(GladeSearchpath* gladePath);
	void initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile, bool failIfNotFound = true);
	string findResourcePath(string searchFile);

private:
	};
