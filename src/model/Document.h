/*
 * Xournal++
 *
 * The document
 *
 * All methods are unlocked, you need to lock the document before you change something and unlock after.
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include <String.h>
#include <XournalType.h>

#include "PageRef.h"
#include "LinkDestination.h"

#include "../pdf/popplerdirect/poppler/XojPopplerDocument.h"
#include "../pdf/popplerdirect/poppler/XojPopplerPage.h"
#include "../pdf/popplerdirect/poppler/XojPopplerIter.h"
#include "../pdf/popplerdirect/poppler/XojPopplerAction.h"

#include "DocumentHandler.h"

#include <vector>

class Document {
public:
	Document(DocumentHandler * handler);
	virtual ~Document();

public:
	bool readPdf(String filename, bool initPages, bool attachToDocument);

	int getPageCount();
	int getPdfPageCount();
	XojPopplerPage * getPdfPage(int page);
	XojPopplerDocument & getPdfDocument();

	void insertPage(PageRef p, int position);
	void addPage(PageRef p);
	PageRef getPage(int page);
	void deletePage(int pNr);

	void setPageSize(PageRef p, double width, double height);

	int indexOf(PageRef page);

	String getLastErrorMsg();

	bool isPdfDocumentLoaded();
	int findPdfPage(int pdfPage);

	void operator=(Document & doc);

	void setFilename(String filename);
	String getFilename();
	String getPdfFilename();

	String getEvMetadataFilename();

	GtkTreeModel * getContentsModel();

	void setCreateBackupOnSave(bool backup);
	bool shouldCreateBackupOnSave();

	void clearDocument(bool destroy = false);

	bool isAttachPdf();

	cairo_surface_t * getPreview();
	void setPreview(cairo_surface_t * preview);

	void lock();
	void unlock();
	bool tryLock();

private:
	void buildContentsModel();
	void buildTreeContentsModel(GtkTreeIter * parent, XojPopplerIter * iter);
	void updateIndexPageNumbers();
	static bool fillPageLabels(GtkTreeModel * tree_model, GtkTreePath * path, GtkTreeIter * iter, Document * doc);

private:
	XOJ_TYPE_ATTRIB;


	DocumentHandler * handler;

	XojPopplerDocument pdfDocument;

	String filename;
	String pdfFilename;
	bool attachPdf;

	/**
	 * Password: not handled yet
	 */
	String password;

	String lastError;

	/**
	 * The pages in the document
	 */
	std::vector<PageRef> pages;

	/**
	 * The bookmark contents model
	 */
	GtkTreeModel * contentsModel;

	/**
	 * create a backup before save, because the original file was an older fileversion
	 */
	bool createBackupOnSave;

	/**
	 * The preview for the file
	 */
	cairo_surface_t * preview;

	/**
	 * The lock of the document
	 */
	GMutex documentLock;
};

#endif /* __DOCUMENT_H__ */
