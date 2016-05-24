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
 * @license GNU GPLv2 or later
 */

#pragma once

#include "DocumentHandler.h"
#include "LinkDestination.h"
#include "PageRef.h"

#include "pdf/popplerdirect/poppler/XojPopplerDocument.h"
#include "pdf/popplerdirect/poppler/XojPopplerPage.h"
#include "pdf/popplerdirect/poppler/XojPopplerIter.h"
#include "pdf/popplerdirect/poppler/XojPopplerAction.h"

#include <StringUtils.h>
#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include <vector>

class Document
{
public:
	Document(DocumentHandler* handler);
	virtual ~Document();

public:
	bool readPdf(path filename, bool initPages, bool attachToDocument);

	size_t getPageCount();
	size_t getPdfPageCount();
	XojPopplerPage* getPdfPage(size_t page);
	XojPopplerDocument& getPdfDocument();

	void insertPage(PageRef p, size_t position);
	void addPage(PageRef p);
	PageRef getPage(size_t page);
	void deletePage(size_t pNr);

	void setPageSize(PageRef p, double width, double height);

	size_t indexOf(PageRef page);

	string getLastErrorMsg();

	bool isPdfDocumentLoaded();
	size_t findPdfPage(size_t pdfPage);

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
	static bool fillPageLabels(GtkTreeModel* tree_model, GtkTreePath* path, GtkTreeIter* iter, Document* doc);

private:
	XOJ_TYPE_ATTRIB;


	DocumentHandler* handler;

	XojPopplerDocument pdfDocument;

	path filename;
	path pdfFilename;
	bool attachPdf;

	/**
	 *  Password: not handled yet
	 */
	string password;

	string lastError;

	/**
	 * The pages in the document
	 */
	std::vector<PageRef> pages;

	/**
	 * The bookmark contents model
	 */
	GtkTreeModel* contentsModel;

	/**
	 *  create a backup before save, because the original file was an older fileversion
	 */
	bool createBackupOnSave;

	/**
	 * The preview for the file
	 */
	cairo_surface_t* preview;

	/**
	 * The lock of the document
	 */
	GMutex documentLock;
};
