/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFBOOKMARK_H__
#define __PDFBOOKMARK_H__

#include <gtk/gtk.h>
#include "../../model/Document.h"
#include "PdfWriter.h"

class PdfBookmarks
{
public:
	PdfBookmarks();
	virtual ~PdfBookmarks();

public:
	void createBookmarks(GtkTreeModel* model, GList*& data, GtkTreeIter* iter,
	                     int level, Document* doc);
	GList* exportBookmarksFromTreeModel(GtkTreeModel* model, Document* doc);
	void writeOutlines(Document* doc, PdfWriter* writer, int* outlineRoot,
	                   GList* pageIds);

private:
	XOJ_TYPE_ATTRIB;
};

#endif /* __PDFBOOKMARK_H__ */
