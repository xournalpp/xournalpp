/*
 * Xournal++
 *
 * Export handler for export as PDF, PNG, EPS... etc.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class GladeSearchpath;
class Settings;
class Control;
class Document;
class ExportDialog;

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
