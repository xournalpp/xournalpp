/*
 * Xournal++
 *
 * The document
 *
 * All methods are unlocked, you need to lock the document before you change something and unlock after.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include <StringUtils.h>
#include <XournalType.h>

#include "PageRef.h"
#include "LinkDestination.h"

#include "../pdf/popplerdirect/poppler/XojPopplerDocument.h"
#include "../pdf/popplerdirect/poppler/XojPopplerPage.h"
#include "../pdf/popplerdirect/poppler/XojPopplerIter.h"
#include "../pdf/popplerdirect/poppler/XojPopplerAction.h"

#include "DocumentHandler.h"

#include <vector>

class Document
{
public:
	Document(DocumentHandler* handler);
	virtual ~Document();

public:
	bool readPdf(path filename, bool initPages, bool attachToDocument);

	int getPageCount();
	int getPdfPageCount();
	XojPopplerPage* getPdfPage(int page);
	XojPopplerDocument& getPdfDocument();

	void insertPage(PageRef p, int position);
	void addPage(PageRef p);
	PageRef getPage(int page);
	void deletePage(int pNr);

	void setPageSize(PageRef p, double width, double height);

	int indexOf(PageRef page);

	string getLastErrorMsg();

	bool isPdfDocumentLoaded();
	int findPdfPage(int pdfPage);

	void operator=(Document& doc);

	void setFilename(path filename);
	path getFilename();
	path getPdfFilename();

	path getEvMetadataFilename();

	GtkTreeModel* getContentsModel();

	void setCreateBackupOnSave(bool backup);
	bool shouldCreateBackupOnSave();

	void clearDocument(bool destroy = false);

	bool isAttachPdf();

	cairo_surface_t* getPreview();
	void setPreview(cairo_surface_t* preview);

	void lock();
	void unlock();
	bool tryLock();

private:
	void buildContentsModel();
	void buildTreeContentsModel(GtkTreeIter* parent, XojPopplerIter* iter);
	void updateIndexPageNumbers();
	static bool fillPageLabels(GtkTreeModel* tree_model, GtkTreePath* path,
							   GtkTreeIter* iter, Document* doc);

private:
	XOJ_TYPE_ATTRIB;


	DocumentHandler* handler;

	XojPopplerDocument pdfDocument;

	path filename;
	path pdfFilename;
	bool attachPdf;

	/** Password: not handled yet */
	string password;

	string lastError;

	/** The pages in the document */
	std::vector<PageRef> pages;

	/** The bookmark contents model */
	GtkTreeModel* contentsModel;

	/** create a backup before save, because the original file was an older fileversion */
	bool createBackupOnSave;

	/** The preview for the file */
	cairo_surface_t* preview;

	/** The lock of the document */
	GMutex documentLock;
};
