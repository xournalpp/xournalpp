/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "PdfWriter.h"
#include "model/Document.h"

#include <gtk/gtk.h>

class PdfBookmarks
{
public:
	PdfBookmarks();
	virtual ~PdfBookmarks();

public:
	void createBookmarks(GtkTreeModel* model, GList*& data, GtkTreeIter* iter, int level, Document* doc);
	GList* exportBookmarksFromTreeModel(GtkTreeModel* model, Document* doc);
	void writeOutlines(Document* doc, PdfWriter* writer, int* outlineRoot, GList* pageIds);

private:
	XOJ_TYPE_ATTRIB;
};
