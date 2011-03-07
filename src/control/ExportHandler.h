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

#include "../model/Document.h"
#include "../gui/dialog/ExportDialog.h"

class GladeSearchpath;
class Settings;

class ExportHandler {
public:
	ExportHandler();
	virtual ~ExportHandler();

public:
	void runExportWithDialog(GladeSearchpath * gladeSearchPath, Settings * settings, Document * doc, int current);

private:
	bool createSurface(int id, double width, double height);
	void freeSurface(int id);

private:
	cairo_surface_t * surface;
	cairo_t * cr;

	int dpi;
	ExportFormtType type;
	String filename;
	String folder;
};

#endif /* __EXPORTHANDLER_H__ */
