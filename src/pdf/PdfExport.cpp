#include "PdfExport.h"
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <string.h>
#include <config.h>
#include <stdlib.h>
#include "../util/GzHelper.h"
#include "cairo/CairoPdf.h"

/**
 * This class uses some inspiration from FPDF (PHP Class)
 */

PdfExport::PdfExport(Document * doc) {
	this->doc = doc;
	this->out = NULL;

	// TODO: enable compression
	this->compressOutput = false;

	this->objectId = 3;
	this->xref = NULL;
	this->xrefLenght = 0;
	this->xrefNr = 0;
	this->dataCount = 0;
	this->dataXrefStart = 0;

	this->pageCount = 0;

	this->inStream = false;
	this->stream = NULL;

	addXref(0);
	addXref(0);

	this->outlineRoot = 0;
}

PdfExport::~PdfExport() {
	g_free(xref);

	if (this->stream) {
		g_string_free(this->stream, true);
	}
}

bool PdfExport::write(const char * data) {
	return writeLen(data, strlen(data));
}

bool PdfExport::writef(const char * format, ...) {
	va_list args;
	va_start (args, format);
	char * data = g_strdup_vprintf(format, args);
	bool res = writeLen(data, strlen(data));
	g_free(data);
	return res;
}

bool PdfExport::write(int data) {
	return writef("%i", data);;
}

bool PdfExport::writeTxt(const char * data) {
	GString * str = g_string_sized_new(strlen(data) + 100);
	g_string_append(str, "(");

	while (*data) {
		if (*data == '\\' || *data == '(' || *data == ')' || *data == '\r') {
			g_string_append_c(str, '\\');
		}
		g_string_append_c(str, *data);
		data++;
	}

	g_string_append_c(str, ')');
	return writeLen(str->str, str->len);

	g_string_free(str, true);
}

bool PdfExport::writeLen(const char * data, int len) {
	if (this->inStream) {
		g_string_append_len(this->stream, data, len);
		return true;
	}

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

void PdfExport::addXref(int ref) {
	if (xrefLenght < xrefNr + 1) {
		xrefLenght += 100;
		this->xref = (int *) g_realloc(this->xref, xrefLenght * sizeof(int));
	}

	this->xref[xrefNr++] = ref;
}

bool PdfExport::writeObj() {
	bool res;
	addXref(this->dataCount);
	res = this->writef("%i 0 obj\n", this->objectId++);
	if (!res) {
		this->lastError = "Internal PDF error #8";
	}

	return res;
}

void PdfExport::startStream() {
	this->inStream = true;
	if (this->stream == NULL) {
		this->stream = g_string_new("");
	}
}

void PdfExport::endStream() {
	this->inStream = false;

	GString * data = NULL;
	GString * compressed = NULL;

	if (this->compressOutput) {
		compressed = GzHelper::gzcompress(this->stream);
	}

	const char * filter = "";
	if (compressed) {
		filter = "/Filter /FlateDecode ";
		data = compressed;
	} else {
		data = this->stream;
	}

	writef("<<%s/Length %i>>\n", filter, this->stream->len);
	write("stream\n");

	writeLen(data->str, data->len);

	write("\nendstream\n");

	if (compressed) {
		g_string_free(compressed, true);
	}

	this->stream->len = 0;
}

bool PdfExport::writeInfo() {
	if (!writeObj()) {
		return false;
	}

	write("<<\n");

	write("/Producer ");
	writeTxt("Xournal++");
	write("\n");

	String title = doc->getFilename();
	if (!title.isEmpty()) {
		write("/Title ");
		if (title.length() > 4 && title.substring(-4, 1) == ".") {
			title = title.substring(0, -4);
		}
		writeTxt(title.c_str());
		write("\n");
	}

	write("/Author ");
	writeTxt(getenv("USERNAME"));
	write("\n");

	write("/Creator ");
	writeTxt("Cairo / Poppler " VERSION);
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

	write("/OpenAction [3 0 R /FitH null]\n");
	//write("/OpenAction [3 0 R /Fit]\n");

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
	XojLinkDest * link = NULL;
	LinkDestination * dest = NULL;

	do {
		gtk_tree_model_get(model, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
		dest = link->dest;

		int page = doc->findPdfPage(dest->getPdfPage());
		if (page == -1) {
			continue;
		}

		data = g_list_append(data, new Bookmark(dest->getName(), level, page, dest->getTop()));

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

bool PdfExport::writePagesindex() {
	this->xref[0] = this->dataCount;
	//Pages root
	write("1 0 obj\n");
	write("<</Type /Pages\n");
	write("/Kids [");

	for (int i = 0; i < this->pageCount; i++) {
		writef("%i 0 R ", 3 + 2 * i);
	}
	write("]\n");
	writef("/Count %i\n", this->pageCount);

	XojPage * page = doc->getPage(0);

	writef("/MediaBox [0 0 %.2F %.2F]\n", page->getWidth(), page->getHeight());
	write(">>\n");
	write("endobj\n");

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

	writef("/Size %i\n", this->objectId);
	writef("/Root %i 0 R\n", this->objectId - 1);
	writef("/Info %i 0 R\n", this->objectId - 2);
	write(">>\n");
	write("startxref\n");

	writef("%i\n", this->dataXrefStart);
	write("%%EOF\n");

	return this->lastError.isEmpty();
}

bool PdfExport::writeXobjectdict() {
	//	foreach($this->images as $image)
	//		$this->_out('/I'.$image['i'].' '.$image['n'].' 0 R');

	return true;
}

bool PdfExport::writeResourcedict() {
	write("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]\n");
	write("/Font <<\n");
	//		foreach($this->fonts as $font)
	//			$this->_out('/F'.$font['i'].' '.$font['n'].' 0 R');
	write(">>\n");
	write("/XObject <<\n");
	if (!writeXobjectdict()) {
		return false;
	}
	write(">>\n");
	return true;
}

bool PdfExport::writeResources() {
	//	$this->_putfonts();
	//	$this->_putimages();
	//Resource dictionary
	this->xref[1] = this->dataCount;

	write("2 0 obj\n");
	write("<<\n");
	if (!writeResourcedict()) {
		return false;
	}
	write(">>\n");
	write("endobj\n");

	return true;
}

bool PdfExport::writeFooter() {
	if (!writePagesindex()) {
		g_warning("failed to write outlines");
		return false;
	}

	if (!writeResources()) {
		g_warning("failed to write resources");
		return false;
	}

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

void printPopplerObjectType(Object * o) {
	ObjType type = o->getType();
	if (type == objBool) {
		printf("pdf::bool: %i\n", o->getBool());
	} else if (type == objInt) {
		printf("pdf::int: %i\n", o->getInt());
	} else if (type == objReal) {
		printf("pdf::int: %lf\n", o->getReal());
	} else if (type == objString) {
		printf("pdf::string: %s\n", o->getString()->getCString());
	} else if (type == objName) {
		printf("pdf::name: %s\n", o->getName());
	} else if (type == objNull) {
		printf("pdf::null\n");
	} else if (type == objArray) {
		printf("pdf::array:\n");
	} else if (type == objDict) {
		printf("pdf::dict:\n");
	} else if (type == objStream) {
		printf("pdf::stream:\n");
	} else if (type == objRef) {
		printf("pdf::ref:\n");
	} else if (type == objCmd) {
		printf("pdf::cmd:\n");
	} else if (type == objError) {
		printf("pdf::error:\n");
	} else if (type == objEOF) {
		printf("pdf::eof:\n");
	} else if (type == objNone) {
		printf("pdf::none:\n");
	}
}

bool PdfExport::addPopplerPage(XojPopplerPage * pdf) {
	Page * page = pdf->getPage();

	Object * o = new Object();
	page->getContents(o);

	ObjType type = o->getType();
	printPopplerObjectType(o);

	if (type != objStream) {
		delete o;
		return false;
	}
	Stream * s = o->getStream();
	s->reset();

	char buffer[512];
	while (s->lookChar() != EOF) {
		int i = 0;
		for (i = 0; i < sizeof(buffer) && s->lookChar() != EOF; i++) {
			buffer[i] = s->getChar();
		}
		writeLen(buffer, i);
	}

	delete o;
	return true;
}

bool PdfExport::writePage(int pageNr) {
	XojPage * page = doc->getPage(pageNr);

	if (!page) {
		return false;
	}

	this->pageCount++;

	writeObj();
	write("<</Type /Page\n");
	write("/Parent 1 0 R\n");

	writef("/MediaBox [0 0 %.2F %.2F]\n", page->getWidth(), page->getHeight());
	write("/Resources 2 0 R\n");
	//	if (isset($this->PageLinks[$n])) {
	//		//Links
	//		$annots = '/Annots [';
	//foreach	($this->PageLinks[$n] as $pl)
	//	{
	//		$rect=sprintf('%.2F %.2F %.2F %.2F',$pl[0],$pl[1],$pl[0]+$pl[2],$pl[1]-$pl[3]);
	//		$annots.='<</Type /Annot /Subtype /Link /Rect ['.$rect.'] /Border [0 0 0] ';
	//		if(is_string($pl[4]))
	//		$annots.='/A <</S /URI /URI '.$this->_textstring($pl[4]).'>>>>';
	//		else
	//		{
	//			$l=$this->links[$pl[4]];
	//			$h=isset($this->PageSizes[$l[0]]) ? $this->PageSizes[$l[0]][1] : $hPt;
	//			$annots.=sprintf('/Dest [%d 0 R /XYZ 0 %.2F null]>>',1+2*$l[0],$h-$l[1]*$this->k);
	//		}
	//	}
	//	$this->_out($annots.']');
	//}
	writef("/Contents %i 0 R>>\n", this->objectId);
	write("endobj\n");
	//Page content
	writeObj();

	startStream();

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		XojPopplerPage * pdf = doc->getPdfPage(page->getPdfPageNr());
		if (!addPopplerPage(pdf)) {
			return false;
		}
	}

	CairoPdf cPdf;
	cPdf.drawPage(page);
	if (!addPopplerPage(cPdf.getPage())) {
		return false;
	}

	endStream();

	write("endobj\n");

	return true;
}

bool PdfExport::createPdf(String uri, bool * cancel) {
	if (doc->getPageCount() < 1) {
		lastError = "No pages to export!";
		return false;
	}

	GError * error = NULL;

	GFile * file = g_file_new_for_uri(uri.c_str());
	this->out = g_file_replace(file, NULL, false, (GFileCreateFlags) 0, NULL, &error);

	g_object_unref(file);

	if (error) {
		lastError = "Error opening file for writing: ";
		lastError += error->message;
		lastError += ", File: ";
		lastError += uri;
		g_warning("error opening file");
		return false;
	}

	write("%PDF-1.4\n");

	for (int i = 0; i < doc->getPageCount(); i++) {
		if (!writePage(i)) {
			g_warning("error writing page %i", i + 1);
			return false;
		}
	}

	// Write our own footer
	if (!writeFooter()) {
		g_warning("error writing footer");
		return false;
	}

	g_output_stream_close(G_OUTPUT_STREAM(this->out), NULL, NULL);

	return true;
}

