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
	static void initLocalisation();

	static void checkForErrorlog();
	static void checkForEmergencySave(Control* control);

	static int exportPdf(const char* input, const char* output);
	static int exportImg(const char* input, const char* output);

	void initSettingsPath();
	void initResourcePath(GladeSearchpath* gladePath);
	static void initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile,
	                             bool failIfNotFound = true);
	static string findResourcePath(const string& searchFile);

private:
};
