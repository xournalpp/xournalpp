#include "PdfExport.h"
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <string.h>
#include <config.h>
#include <stdlib.h>
#include "../view/DocumentView.h"

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
const char * strnstr(const char *s, const char *find, size_t slen) {
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *) s);
}

PdfExport::PdfExport(Document * doc) {
	this->doc = doc;
	this->out = NULL;
	this->inPagesEnd = false;
	this->inSavestream = false;
	this->inFooter = false;
	this->end = g_string_new_len("", 1024);
	this->end->len = 0;

	this->objectId = 0;
	this->xref = NULL;
	this->xrefLenght = 0;
	this->xrefNr = 0;
	this->dataCount = 0;
	this->dataXrefStart = 0;

	this->outlineRoot = 0;
}

PdfExport::~PdfExport() {
	if (this->out) {
		g_object_unref(this->out);
	}

	g_string_free(this->end, true);

	g_free(xref);
}

bool PdfExport::write(const char * data) {
	return write(data, strlen(data));
}

bool PdfExport::write(int data) {
	char * str = g_strdup_printf("%i", data);
	bool res = write(str);
	g_free(str);

	return res;
}

bool PdfExport::writeTxt(const char * data) {
	GString * str = g_string_new_len("(", strlen(data) + 100);
	str->len = 1;

	while (*data) {
		if (*data == '\\' || *data == '(' || *data == ')' || *data == '\r') {
			g_string_append_c(str, '\\');
		}
		g_string_append_c(str, *data);
		data++;
	}

	g_string_append_c(str, ')');
	return write(str->str, str->len);

	g_string_free(str, true);
}

bool PdfExport::write(const char * data, int len) {
	GError * err = NULL;

	g_output_stream_write(G_OUTPUT_STREAM(this->out), data, len, NULL, &err);

	this->dataCount += len;

	if (err) {
		this->lastError = "Error writing stream: ";
		this->lastError += err->message;

		printf("error writing file: %s\n", err->message);
		g_error_free(err);
		return false;
	}

	return true;
}

cairo_status_t PdfExport::writeOut(PdfExport *pdf, unsigned char *data, unsigned int length) {
	unsigned char * tmp = NULL;

	if (pdf->inSavestream) {
		g_string_append_len(pdf->end, (char *) data, length);
		return CAIRO_STATUS_SUCCESS;
	}

	if (pdf->inFooter) {
		if (strnstr((char *) data, "<< /Type /Pages\n", length) != NULL) {
			pdf->inPagesEnd = true;
		} else if (pdf->inPagesEnd && strnstr((char *) data, "endobj", length) != NULL) {
			pdf->inPagesEnd = false;

			pdf->inSavestream = true;
		}
	}

	if (!pdf->write((char*) data, length)) {
		return CAIRO_STATUS_WRITE_ERROR;
	}

	return CAIRO_STATUS_SUCCESS;
}

void PdfExport::addXref(int ref) {
	if (xrefLenght < xrefNr + 1) {
		xrefLenght += 100;
		this->xref = (int *) g_realloc(this->xref, xrefLenght * sizeof(int));
	}

	this->xref[xrefNr++] = ref;
}

bool PdfExport::parseFooter() {
	char * ptr = NULL;

	this->objectId = strtol(this->end->str, &ptr, 10);
	if (ptr == this->end->str) {
		this->lastError = "Internal PDF error #1";
		return false;
	}

	const char * xref = strnstr(this->end->str, "xref", this->end->len);

	if (xref == NULL) {
		this->lastError = "Internal PDF error #2";
		return false;
	}

	// End of line
	xref = strnstr(xref, "\n", this->end->len) + 1;

	if (xref == NULL) {
		this->lastError = "Internal PDF error #3";
		return false;
	}

	// End of object id
	xref = strnstr(xref, "\n", this->end->len) + 1;

	if (xref == NULL) {
		this->lastError = "Internal PDF error #4";
		return false;
	}

	// End of 0000000000 65535 f
	xref = strnstr(xref, "\n", this->end->len);

	if (xref == NULL) {
		this->lastError = "Internal PDF error #5";
		return false;
	}

	const char * xrefEnd = strnstr(xref, "trailer\n", this->end->len);

	if (xrefEnd == NULL) {
		this->lastError = "Internal PDF error #6";
		return false;
	}

	for (const char * i = xref; i < xrefEnd; i++) {
		int addr = strtol(i, &ptr, 10);
		if (ptr == i) {
			this->lastError = "Internal PDF error #7";
			return false;
		}
		addXref(addr);

		//skip the rest of the line
		while (*i != '\n') {
			i++;
		}
	}

	return true;
}

bool PdfExport::writeObj() {
	bool res;
	char * obj = g_strdup_printf("%i 0 obj\n", this->objectId++);
	res = this->write(obj);
	g_free(obj);

	addXref(this->dataCount);

	if (!res) {
		this->lastError = "Internal PDF error #8";
	}

	return res;
}

bool PdfExport::writeInfo() {
	if (!writeObj()) {
		return false;
	}

	write("<<\n");

	write("/Producer ");
	char * producer = g_strdup_printf("cairo %s (http://cairographics.org)", cairo_version_string());
	writeTxt(producer);
	g_free(producer);
	write("\n");

	//		write("/Title ");
	//		writeTxt(doc->getFilename());
	//		write("\n");

	write("/Author ");
	writeTxt(getenv("USERNAME"));
	write("\n");

	write("/Creator ");
	writeTxt("Xournal++ " VERSION);
	write("\n");

	time_t curtime = time(NULL);
	char stime[128] = "D:";
	strftime(stime + 2, sizeof(stime) - 2, "%Y%m%d%H%M%S", localtime(&curtime));

	write("/CreationDate ");
	writeTxt(stime);
	write("\n");

	write(">>\nendobj\n");

	return this->lastError.isEmpty();
}

bool PdfExport::writeCatalog() {
	if (!writeObj()) {
		return false;
	}

	write("<<\n");

	write("/Type /Catalog\n");
	write("/Pages 1 0 R\n");

	write("/OpenAction [3 0 R /Fit]\n");

	//	if($this->ZoomMode=='fullpage')
	//		$this->_out('/OpenAction [3 0 R /Fit]');
	//	elseif($this->ZoomMode=='fullwidth')
	//		$this->_out('/OpenAction [3 0 R /FitH null]');
	//	elseif($this->ZoomMode=='real')
	//		$this->_out('/OpenAction [3 0 R /XYZ null null 1]');
	//	elseif(!is_string($this->ZoomMode))
	//		$this->_out('/OpenAction [3 0 R /XYZ null null '.($this->ZoomMode/100).']');

	write("/PageLayout /OneColumn\n");

	//	if($this->LayoutMode=='single')
	//		$this->_out('/PageLayout /SinglePage');
	//	elseif($this->LayoutMode=='continuous')
	//		$this->_out('/PageLayout /OneColumn');
	//	elseif($this->LayoutMode=='two')
	//		$this->_out('/PageLayout /TwoColumnLeft');

	if (this->outlineRoot) {
		char * outline = g_strdup_printf("/Outlines %i 0 R\n", this->outlineRoot);
		write(outline);
		g_free(outline);
		write("/PageMode /UseOutlines\n");
	}

	write(">>\nendobj\n");

	return this->lastError.isEmpty();
}

bool PdfExport::writeCrossRef() {
	this->dataXrefStart = this->dataCount;
	write("xref\n");
	write("0 ");
	char * tmp = g_strdup_printf("%i", this->objectId);
	write(tmp);
	g_free(tmp);
	write("\n");

	write("0000000000 65535 f \n");

	char buffer[64];

	for (int i = 0; i < this->xrefNr; i++) {
		sprintf(buffer, "%010d 00000 n \n", this->xref[i]);
		write(buffer);
	}

	return this->lastError.isEmpty();
}

class Bookmark {
public:
	Bookmark(String name, int level, int page, int top) {
		this->name = name;
		this->level = level;
		this->parent = -1;
		this->last = -1;
		this->first = -1;
		this->prev = -1;
		this->next = -1;
		this->page = page;
		this->top = top;
	}

	String name;
	int level;

	int parent;
	int last;
	int first;
	int next;
	int prev;
	int page;
	int top;
};

void PdfExport::createBookmarks(GtkTreeModel * model, GList * &data, GtkTreeIter * iter, int level) {
	LinkDest * link = NULL;
	LinkDestination * dest = NULL;

	do {
		gtk_tree_model_get(model, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
		dest = link->dest;

		data = g_list_append(data, new Bookmark(dest->getName(), level, dest->getPage(), dest->getTop()));

		GtkTreeIter children = { 0 };

		if (gtk_tree_model_iter_children(model, &children, iter)) {
			createBookmarks(model, data, &children, level + 1);
		}
	} while (gtk_tree_model_iter_next(model, iter));
}

GList * PdfExport::exportBookmarksFromTreeModel(GtkTreeModel * model) {
	GList * data = NULL;
	GtkTreeIter iter = { 0 };

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		return false;
	}

	createBookmarks(model, data, &iter, 0);

	return data;
}

bool PdfExport::writeOutlines() {
	GtkTreeModel * model = doc->getContentsModel();

	if (!model) {
		return true;
	}

	GList * bookmarkList = exportBookmarksFromTreeModel(model);
	int bookmarksLenght = g_list_length(bookmarkList);
	Bookmark ** bookmarks = new Bookmark *[bookmarksLenght];

	int maxLevel = 0;
	int i = 0;
	for (GList * l = bookmarkList; l != NULL; l = l->next) {
		Bookmark * b = (Bookmark *) l->data;
		if (maxLevel < b->level) {
			maxLevel = b->level;
		}
		bookmarks[i++] = b;
	}

	g_list_free(bookmarkList);

	int * levels = new int[maxLevel + 1];
	int level = 0;

	for (i = 0; i < bookmarksLenght; i++) {
		Bookmark * b = bookmarks[i];

		if (b->level > 0) {
			int parent = levels[b->level - 1];
			//Set parent and last pointers
			b->parent = parent;
			bookmarks[parent]->last = i;
			if (b->level > level) {
				//Level increasing: set first pointer
				bookmarks[parent]->first = i;
			}
		} else {
			b->parent = bookmarksLenght;
		}

		if (b->level <= level && i > 0) {
			//Set prev and next pointers
			int prev = levels[b->level];
			bookmarks[prev]->next = i;
			b->prev = prev;
		}

		levels[b->level] = i;
		level = b->level;
		i++;
	}

	//Outline items
	char buffer[256];
	int n = this->objectId;

	for (i = 0; i < bookmarksLenght; i++) {
		Bookmark * b = bookmarks[i];
		if (!writeObj()) {
			return false;
		}

		write("<<\n/Title ");
		writeTxt(b->name.c_str());
		write("\n");

		write("/Parent ");
		write(n + b->parent);
		write(" 0 R\n");

		if (b->prev != -1) {
			write("/Prev ");
			write(n + b->prev);
			write(" 0 R\n");
		}

		if (b->next != -1) {
			write("/Next ");
			write(n + b->next);
			write(" 0 R\n");
		}

		if (b->first != -1) {
			write("/First ");
			write(n + b->first);
			write(" 0 R\n");
		}

		if (b->last != -1) {
			write("/Last ");
			write(n + b->last);
			write(" 0 R\n");
		}

		XojPage * p = doc->getPage(b->page);
		float top = 0;
		if (p != NULL) {
			top = (p->getHeight() - b->top) * 72;
		}

		sprintf(buffer, "/Dest [%d 0 R /XYZ 0 %.2f null]", 1 + 2 * b->page, top /*($this->h-$o['y'])*$this->k*/);

		write(buffer);
		write("/Count 0\n>>\n");
		write("endobj\n");
	}

	//Outline root
	writeObj();
	this->outlineRoot = this->objectId - 1;
	write("<<\n/Type /Outlines /First ");
	write(n);
	write(" 0 R\n");
	write("/Last ");
	write(n + levels[0]);
	write(" 0 R\n>>\n");
	write("endobj\n");

	for (i = 0; i < bookmarksLenght; i++) {
		delete bookmarks[i];
	}

	delete[] bookmarks;
	delete[] levels;

	return true;
}

bool PdfExport::writeTrailer() {
	write("trailer\n");
	write("<<\n");

	char * tmp;

	tmp = g_strdup_printf("/Size %i\n", this->objectId);
	write(tmp);
	g_free(tmp);

	tmp = g_strdup_printf("/Root %i 0 R\n", this->objectId - 1);
	write(tmp);
	g_free(tmp);

	tmp = g_strdup_printf("/Info %i 0 R\n", this->objectId - 2);
	write(tmp);
	g_free(tmp);

	write(">>\n");
	write("startxref\n");

	char * startxref = g_strdup_printf("%i\n", this->dataXrefStart);
	write(startxref);
	g_free(startxref);
	write("%%EOF\n");

	return this->lastError.isEmpty();
}

bool PdfExport::writeFooter() {
	if (!writeOutlines()) {
		g_warning("failed to write outlines");
		return false;
	}

	if (!writeInfo()) {
		g_warning("failed to write info");
		return false;
	}

	if (!writeCatalog()) {
		g_warning("failed to write catalog");
		return false;
	}

	if (!writeCrossRef()) {
		g_warning("failed to write cross ref");
		return false;
	}

	if (!writeTrailer()) {
		g_warning("failed to write trailer");
		return false;
	}

	return true;
}

String PdfExport::getLastError() {
	return lastError;
}

bool PdfExport::createPdf(String uri, bool * cancel) {
	GError * error = NULL;

	GFile * file = g_file_new_for_uri(uri.c_str());
	this->out = g_file_replace(file, NULL, false, G_FILE_CREATE_NONE, NULL, &error);

	g_object_unref(file);

	if (error) {
		lastError = "Error opening file for writing: ";
		lastError += error->message;
		g_warning("error opening file");
		return false;
	}

	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_pdf_surface_create_for_stream((cairo_write_func_t) writeOut, this, 504, 648);

	cr = cairo_create(surface);

	DocumentView view;

	for (int i = 0; i < doc->getPageCount() && !*cancel; i++) {
		XojPage * page = doc->getPage(i);

		PopplerPage * popplerPage = NULL;

//		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
//			int pgNo = page->getPdfPageNr();
//			popplerPage = doc->getPdfPage(pgNo);
//		}

		cairo_pdf_surface_set_size(surface, page->getWidth(), page->getHeight());

		view.drawPage(page, popplerPage, cr);

		cairo_show_page(cr);

		printf("page: %i\n", i);
	}

	this->inFooter = true;

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	for (int i = 0; i < this->end->len; i++) {
		printf("%c", this->end->str[i]);
	}

	// Parse existing PDF Footer
	if (!parseFooter()) {
		g_warning("error parsing footer");
		return false;
	}

	this->dataCount = xref[this->objectId];
	this->xrefNr = this->objectId;

	// Write our own footer
	if (!writeFooter()) {
		g_warning("error writing footer");
		return false;
	}

	g_output_stream_close(G_OUTPUT_STREAM(this->out), NULL, NULL);

	return true;
}

