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

#include "pdf/base/XojPdfDocument.h"
#include "pdf/base/XojPdfPage.h"
#include "pdf/base/XojPdfBookmarkIterator.h"

#include <Path.h>
#include <XournalType.h>

class Document
{
public:
	Document(DocumentHandler* handler);
	virtual ~Document();

public:
	enum DocumentType
	{
		XOPP,
		XOJ,
		PDF
	};

	bool readPdf(Path filename, bool initPages, bool attachToDocument, gpointer data = nullptr, gsize length = 0);

	size_t getPageCount();
	size_t getPdfPageCount();
	XojPdfPageSPtr getPdfPage(size_t page);
	XojPdfDocument& getPdfDocument();

	void insertPage(const PageRef& p, size_t position);
	void addPage(const PageRef& p);
	PageRef getPage(size_t page);
	void deletePage(size_t pNr);

	void setPageSize(PageRef p, double width, double height);
	double getPageWidth(PageRef p);
	double getPageHeight(PageRef p);

	size_t indexOf(const PageRef& page);

	/**
	 * @return The last error message to show to the user
	 */
	string getLastErrorMsg();

	bool isPdfDocumentLoaded();
	size_t findPdfPage(size_t pdfPage);

	Document& operator=(const Document& doc);

	void setFilename(Path filename);
	Path getFilename();
	Path getPdfFilename();
	Path createSaveFolder(Path lastSavePath);
	Path createSaveFilename(DocumentType type, string defaultSaveName);

	Path getEvMetadataFilename();

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
	void freeTreeContentModel();
	static bool freeTreeContentEntry(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc);

	void buildTreeContentsModel(GtkTreeIter* parent, XojPdfBookmarkIterator* iter);
	void updateIndexPageNumbers();
	static bool fillPageLabels(GtkTreeModel* tree_model, GtkTreePath* path, GtkTreeIter* iter, Document* doc);

private:
	DocumentHandler* handler = nullptr;

	XojPdfDocument pdfDocument;

	Path filename;
	Path pdfFilename;
	bool attachPdf = false;

	/**
	 *  Password: not handled yet
	 */
	string password;

	string lastError;

	/**
	 * The pages in the document
	 */
	vector<PageRef> pages;

	/**
	 * The bookmark contents model
	 */
	GtkTreeModel* contentsModel = nullptr;

	/**
	 *  create a backup before save, because the original file was an older fileversion
	 */
	bool createBackupOnSave = false;

	/**
	 * The preview for the file
	 */
	cairo_surface_t* preview = nullptr;

	/**
	 * The lock of the document
	 */
	GMutex documentLock;
};
