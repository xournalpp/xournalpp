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

#include "../util/String.h"
#include "../util/XournalType.h"

#include "Page.h"
#include "LinkDestination.h"

#include "../pdf/poppler/XojPopplerDocument.h"
#include "../pdf/poppler/XojPopplerPage.h"
#include "../pdf/poppler/XojPopplerIter.h"
#include "../pdf/poppler/XojPopplerAction.h"

#include "DocumentHandler.h"

class Document : public MemoryCheckObject {
public:
	Document(DocumentHandler * handler);
	virtual ~Document();

public:
	bool readPdf(String filename, bool initPages, bool attachToDocument);

	int getPageCount();
	int getPdfPageCount();
	XojPopplerPage * getPdfPage(int page);
	XojPopplerDocument & getPdfDocument();

	void insertPage(XojPage * p, int position);
	void addPage(XojPage * p);
	XojPage * getPage(int page);
	void deletePage(int pNr);

	void setPageSize(XojPage * p, double width, double height);

	void firePageSizeChanged(int page);

	int indexOf(XojPage * page);

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

	XojPage ** pages;
	int pageCount;
	int pagesArrayLen;

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
	GMutex * documentLock;
};

#endif /* __DOCUMENT_H__ */
