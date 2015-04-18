/*
 * Xournal++
 *
 * Export handler for export as PDF, PNG, EPS... etc.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

class GladeSearchpath;
class Settings;
class Control;
class Document;
class ExportDialog;

#include <XournalType.h>

class ExportHandler
{
public:
	ExportHandler();
	virtual ~ExportHandler();

public:
	void runExportWithDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
							Document* doc, Control* control, int current);

private:
	XOJ_TYPE_ATTRIB;

};
