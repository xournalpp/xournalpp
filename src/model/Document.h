/*
 * Xournal++
 *
 * The document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include "../util/String.h"

#include "Page.h"
#include "LinkDestination.h"

#include "../pdf/poppler/XojPopplerDocument.h"
#include "../pdf/poppler/XojPopplerPage.h"
#include "../pdf/poppler/XojPopplerIter.h"
#include "../pdf/poppler/XojPopplerAction.h"

class DocumentListener;

enum DocumentChangeType {
	DOCUMENT_CHANGE_CLEARED, DOCUMENT_CHANGE_COMPLETE, DOCUMENT_CHANGE_PDF_BOOKMARKS
};

class DocumentHandler {
public:
	DocumentHandler();
	~DocumentHandler();

	void fireDocumentChanged(DocumentChangeType type);
	void firePageSizeChanged(int page);
	void firePageChanged(int page);
	void firePageInserted(int page);
	void firePageDeleted(int page);
	void firePageLoaded(XojPage * page);
	void firePageSelected(int page);
private:
	void addListener(DocumentListener * l);
	void removeListener(DocumentListener * l);

	GList * listener;

	friend class DocumentListener;
};

class DocumentListener {
public:
	DocumentListener();
	~DocumentListener();

	void registerListener(DocumentHandler * handler);
	void unregisterListener();

	virtual void documentChanged(DocumentChangeType type) = 0;
	virtual void pageSizeChanged(int page) = 0;
	virtual void pageChanged(int page) = 0;
	virtual void pageInserted(int page) = 0;
	virtual void pageDeleted(int page) = 0;
	virtual void pageSelected(int page) = 0;
private:
	DocumentHandler * handler;
};

class Document : public MemoryCheckObject {
public:
	Document(DocumentHandler * handler);
	virtual ~Document();

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

	const char * getEvMetadataFilename();

	GtkTreeModel * getContentsModel();

	void setCreateBackupOnSave(bool backup);
	bool shouldCreateBackupOnSave();

	void clearDocument(bool destroy = false);

	bool isAttachPdf();

	cairo_surface_t * getPreview();
	void setPreview(cairo_surface_t * preview);
private:

	void buildContentsModel();
	void buildTreeContentsModel(GtkTreeIter *parent, XojPopplerIter *iter);
	void updateIndexPageNumbers();
	static bool fillPageLabels(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, Document *doc);

private:
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
};

#endif /* __DOCUMENT_H__ */
