/*
 * Xournal++
 *
 * Export handler for export as PDF, PNG, EPS... etc.
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __EXPORTHANDLER_H__
#define __EXPORTHANDLER_H__

class GladeSearchpath;
class Settings;
class Control;
class Document;
class ExportDialog;

#include <XournalType.h>

class ExportHandler {
public:
	ExportHandler();
	virtual ~ExportHandler();

public:
	void runExportWithDialog(GladeSearchpath * gladeSearchPath, Settings * settings, Document * doc, Control * control, int current);

private:
	XOJ_TYPE_ATTRIB;

};

#endif /* __EXPORTHANDLER_H__ */
