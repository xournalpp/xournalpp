/*
 * Xournal++
 *
 * Xournal main entry, commandline parser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <config.h>
#include <XournalType.h>

class GladeSearchpath;

class XournalMain
{
public:
	XournalMain();
	virtual ~XournalMain();

public:
	int run(int argc, char* argv[]);

private:
#ifdef ENABLE_NLS
	void initLocalisation();
#endif

	void checkForErrorlog();
	void checkForEmergencySave();

	int exportPdf(const char* input, const char* output);
	GladeSearchpath* initPath(const char* argv0);

private:
	XOJ_TYPE_ATTRIB;

};
