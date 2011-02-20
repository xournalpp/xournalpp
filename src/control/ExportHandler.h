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

class ExportHandler {
public:
	ExportHandler();
	virtual ~ExportHandler();

public:
	void runExportWithDialog(Document * doc, int current);

};

#endif /* __EXPORTHANDLER_H__ */
