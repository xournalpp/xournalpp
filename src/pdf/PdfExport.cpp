#include "PdfExport.h"
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <string.h>
#include <config.h>
#include <stdlib.h>
#include "../util/GzHelper.h"
#include "cairo/CairoPdf.h"

#include "PdfExportImage.h"
#include "PdfFont.h"

// A '1' in this array means the character is white space.  A '1' or
// '2' means the character ends a name or command.
static const char specialChars[256] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, // 0x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1x
		1, 0, 0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 2, // 2x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, // 3x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, // 5x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, // 7x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9x
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // ax
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // bx
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // cx
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // dx
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // ex
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // fx
		};

void destroyDelete(gpointer data) {
	delete data;
}

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
	this->images = NULL;

	this->fonts = NULL;
	this->fontId = 1;

	this->documents = NULL;

	this->resources = NULL;

	this->updatedReferenced = g_hash_table_new_full((GHashFunc) UpdateRefKey::hashFunction,
			(GEqualFunc) UpdateRefKey::equalFunction, (GDestroyNotify) destroyDelete, (GDestroyNotify) destroyDelete);

	addXref(0);
	addXref(0);

	this->outlineRoot = 0;
}

PdfExport::~PdfExport() {
	g_free(xref);

	if (this->stream) {
		g_string_free(this->stream, true);
	}

	for (GList * l = this->images; l != NULL; l = l->next) {
		PdfExportImage * img = (PdfExportImage *) l->data;
		delete img;
	}
	g_list_free(this->images);

	for (GList * l = this->documents; l != NULL; l = l->next) {
		XojPopplerDocument * doc = (XojPopplerDocument *) l->data;
		delete doc;
	}
	g_list_free(this->documents);

	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;
		delete font;
	}
	g_list_free(this->fonts);

	g_hash_table_destroy(this->updatedReferenced);

	this->fonts = NULL;
	this->images = NULL;
	this->documents = NULL;
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
	int i = 1;
	for (GList * l = this->images; l != NULL; l = l->next) {
		PdfExportImage * img = (PdfExportImage *) l->data;
		writef("/I%i %i 0 R\n", i, img->getObjectId());

		i++;
	}

	return true;
}

bool PdfExport::writeResourcedict() {
	write("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]\n");
	write("/Font <<\n");

	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;
		writef("/F%i %i 0 R\n", font->id, font->objectId);
	}
	write(">>\n");
	write("/XObject <<\n");
	if (!writeXobjectdict()) {
		return false;
	}
	write(">>\n");
	return true;
}

//void PdfExport::writeUpdateDict(Dict * dict, XojPopplerDocument doc) {
//	for (int i = 0; i < dict->getLength(); i++) {
//		Object o;
//		dict->getValNF(i, &o);
//
//		if (o.isArray()) {
//			Array * arr = o.getArray();
//			for (int u = 0; u < arr->getLength(); u++) {
//				Object o2;
//				arr->getNF(u, &o2);
//				if (o2.isRef()) {
////					Ref ref = o2.getRef();
////					Object data;
////					dict->getVal(i, &data);
////					g_hash_table_insert(this->updatedReferenced, new UpdateRefKey(ref, doc), new UpdateRef(
////							this->objectId));
////					writeObj();
////					writeObject(&data, doc);
////					write("\nendobj\n");
//				} else if (o2.isDict()) {
////					writeUpdateDict(o2.getDict(), doc);
//				} else {
//					printf("array entry type: %i\n", o2.getType());
//				}
//			}
//		} else if (o.isRef()) {
//			Ref ref = o.getRef();
//			Object data;
//			dict->getVal(i, &data);
//			g_hash_table_insert(this->updatedReferenced, new UpdateRefKey(ref, doc), new UpdateRef(this->objectId));
//			writeObj();
//			writeObject(&data, doc);
//			write("\nendobj\n");
//		}
//	}
//}

bool PdfExport::writeFonts() {
	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;

		writeObj();
		font->objectId = this->objectId - 1;
		writeObject(font->object, font->doc);
		write("\nendobj\n");
	}

	bool allWritten = false;
	while (!allWritten) {
		allWritten = true;
		for (GList * l = g_hash_table_get_values(this->updatedReferenced); l != NULL; l = l->next) {
			UpdateRef * uref = (UpdateRef *) l->data;
			if (!uref->wroteOut) {
				this->xref[uref->objectId - 1] = this->dataCount;
				this->writef("%i 0 obj\n", uref->objectId);
				writeObject(&uref->object, uref->doc);
				write("endobj\n");
				uref->wroteOut = true;
				break;
			}
		}
		for (GList * l = g_hash_table_get_values(this->updatedReferenced); l != NULL; l = l->next) {
			UpdateRef * uref = (UpdateRef *) l->data;
			if (!uref->wroteOut) {
				allWritten = false;
			}
		}
	}

	return true;
}

bool PdfExport::writeImages() {
	return true;
}

bool PdfExport::writeResources() {
	if (!writeFonts()) {
		return false;
	}
	if (!writeImages()) {
		return false;
	}

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

void PdfExport::writeObject(Object* obj, XojPopplerDocument doc) {
	Array *array;
	Object obj1;
	int tmp;

	switch (obj->getType()) {
	case objBool:
		writef("%s ", obj->getBool() ? "true" : "false");
		break;
	case objInt:
		writef("%i ", obj->getInt());
		break;
	case objReal:
		writef("%g ", obj->getReal());
		break;
	case objString:
		writeString(obj->getString());
		break;
	case objName: {
		GooString name(obj->getName());
		GooString *nameToPrint = name.sanitizedName(gFalse /* non ps mode */);
		writef("/%s ", nameToPrint->getCString());
		delete nameToPrint;
		break;
	}
	case objNull:
		write("null");
		break;
	case objArray:
		array = obj->getArray();
		write("[");
		for (int i = 0; i < array->getLength(); i++) {
			writeObject(array->getNF(i, &obj1), doc);
			obj1.free();
		}
		write("] ");
		break;
	case objDict:
		writeDictionnary(obj->getDict(), doc);
		break;
	case objStream: {
		// Poppler: We can't modify stream with the current implementation (no write functions in Stream API)
		// Poppler: => the only type of streams which that have been modified are internal streams (=strWeird)
		Stream *stream = obj->getStream();
		if (stream->getKind() == strWeird) {
			//we write the stream unencoded
			stream->reset();
			//recalculate stream length
			tmp = 0;
			for (int c = stream->getChar(); c != EOF; c = stream->getChar()) {
				tmp++;
			}
			obj1.initInt(tmp);
			stream->getDict()->set("Length", &obj1);

			//Remove Stream encoding
			stream->getDict()->remove("Filter");
			stream->getDict()->remove("DecodeParms");

			writeDictionnary(stream->getDict(), doc);
			writeStream(stream);
			obj1.free();
		} else {
			//raw stream copy
			writeDictionnary(stream->getDict(), doc);
			writeRawStream(stream, doc);
		}
		break;
	}
	case objRef: {
		UpdateRefKey key(obj->getRef(), doc);
		UpdateRef * uref = (UpdateRef *) g_hash_table_lookup(this->updatedReferenced, &key);
		if (uref) {
			printf("->ref updated\n");
			writef("%i %i R ", uref->objectId, 0);
		} else {
			UpdateRef * uref = new UpdateRef(this->objectId++, doc);
			addXref(0);

			obj->fetch(doc.getDoc()->getXRef(), &uref->object);

			g_hash_table_insert(this->updatedReferenced, new UpdateRefKey(obj->getRef(), doc), uref);
		}
	}
		break;
	case objCmd:
		write("cmd\r\n");
		break;
	case objError:
		write("error\r\n");
		break;
	case objEOF:
		write("eof\r\n");
		break;
	case objNone:
		write("none\r\n");
		break;
	default:
		g_error("Unhandled objType : %i, please report a bug with a testcase\r\n", obj->getType());
		break;
	}
}

void PdfExport::writeRawStream(Stream* str, XojPopplerDocument doc) {
	Object obj1;
	str->getDict()->lookup("Length", &obj1);
	if (!obj1.isInt()) {
		error(-1, "PDFDoc::writeRawStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();
	obj1.free();

	write("stream\r\n");
	str->unfilteredReset();
	for (int i = 0; i < length; i++) {
		int c = str->getUnfilteredChar();
		writef("%c", c);
	}
	str->reset();
	write("\nendstream\n");
}

void PdfExport::writeString(GooString* s) {
	if (s->hasUnicodeMarker()) {
		//unicode string don't necessary end with \0
		const char* c = s->getCString();
		write("(");
		for (int i = 0; i < s->getLength(); i++) {
			char unescaped = *(c + i) & 0x000000ff;
			//escape if needed
			if (unescaped == '(' || unescaped == ')' || unescaped == '\\') {
				writef("%c", '\\');
			}
			writef("%c", unescaped);
		}
		write(") ");
	} else {
		const char* c = s->getCString();
		write("(");
		while (*c != '\0') {
			char unescaped = (*c) & 0x000000ff;
			//escape if needed
			if (unescaped == '(' || unescaped == ')' || unescaped == '\\') {
				writef("%c", '\\');
			}
			writef("%c", unescaped);
			c++;
		}
		write(") ");
	}
}

void PdfExport::writeDictionnary(Dict* dict, XojPopplerDocument doc) {
	Object obj1;
	write("<<");
	for (int i = 0; i < dict->getLength(); i++) {
		GooString keyName(dict->getKey(i));
		GooString *keyNameToPrint = keyName.sanitizedName(gFalse /* non ps mode */);
		writef("/%s ", keyNameToPrint->getCString());
		delete keyNameToPrint;
		writeObject(dict->getValNF(i, &obj1), doc);
		obj1.free();
	}
	write(">> ");
}

void PdfExport::writeStream(Stream* str) {
	write("stream\r\n");
	str->reset();
	for (int c = str->getChar(); c != EOF; c = str->getChar()) {
		writef("%c", c);
	}
	write("\r\nendstream\r\n");
}

void PdfExport::writeGzStream(Stream* str) {
	Object obj1;
	str->getDict()->lookup("Length", &obj1);
	if (!obj1.isInt()) {
		g_error("PDFDoc::writeGzStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();
	obj1.free();

	char * buffer = new char[length];

	str->unfilteredReset();
	for (int i = 0; i < length; i++) {
		int c = str->getUnfilteredChar();
		buffer[i] = c;
	}
	GString * text = GzHelper::gzuncompress(buffer, length);
	writeStreamLine(text->str, text->len);

	g_string_free(text, true);

	delete buffer;

	str->reset();
}

int PdfExport::lookupFont(String name) {
	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;
		if (font->doc == currentPdfDoc && font->originalName == name) {
			return font->id;
		}
	}

	Object fonts;
	this->resources->lookup("Font", &fonts);

	if (!fonts.isDict()) {
		g_warning("PdfExport::Font is not a dictionary!");
		return 1;
	}

	Dict * d = fonts.getDict();
	Object * f = new Object();

	char * tmp = g_strdup(name.c_str());
	d->lookup(tmp, f);
	g_free(tmp);

	PdfFont * font = new PdfFont(currentPdfDoc, name, fontId++, f);
	this->fonts = g_list_append(this->fonts, font);

	return font->id;
}

void PdfExport::writeStreamLine(char * line, int len) {
	bool inBt = false;
	int start = 0;

	for (int i = 0; i < len - 1; i++) {
		bool lastCharWhitespace = i == 0 || isWhitespace(line[i - 1]);

		if (!inBt && lastCharWhitespace && line[i] == 'B' && line[i + 1] == 'T') {
			inBt = true;
			i++;
		} else if (inBt && lastCharWhitespace && line[i] == '/' && (line[i + 1] == 'f' || line[i + 1] == 'F')) {
			i++;
			writeLen(line + start, i - start);
			start = i;

			int p = 0;
			char buffer[512];

			while (i < len && p < 511 && line[i] && !isWhitespace(line[i])) {
				buffer[p++] = line[i++];
			}
			buffer[p] = 0;
			start = i;

			int font = lookupFont(buffer);
			writef("F%i", font);

			inBt = false;
		} else if (inBt && lastCharWhitespace && line[i] == 'T' && (line[i + 1] == 'f' || line[i + 1] == 'F')) {
			inBt = false;
		} else if (inBt && lastCharWhitespace && line[i] == 'E' && line[i + 1] == 'T') {
			inBt = false;
		}
	}

	writeLen(line + start, len - start);
	printf("->%s\n", line);
}

void PdfExport::writePlainStream(Stream* str) {
	Object obj1;
	str->getDict()->lookup("Length", &obj1);
	if (!obj1.isInt()) {
		g_error("PDFDoc::writeRawStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();
	obj1.free();

	str->unfilteredReset();

	char buffer[1024];
	int x = 0;

	for (int i = 0; i < length; i++) {
		int c = str->getUnfilteredChar();
		buffer[x++] = c;

		if (c == '\n' || x == 1023 || (x > 800 && c == ' ')) {
			buffer[x] = 0;
			writeStreamLine(buffer, x);
			x = 0;
		}
	}
	str->reset();
}

void PdfExport::addPopplerDocument(XojPopplerDocument doc) {
	for (GList * l = this->documents; l != NULL; l = l->next) {
		XojPopplerDocument * d = (XojPopplerDocument *) l->data;
		if (*d == doc) {
			return;
		}
	}

	XojPopplerDocument * d = new XojPopplerDocument();
	*d = doc;

	this->documents = g_list_append(this->documents, d);
}

bool PdfExport::addPopplerPage(XojPopplerPage * pdf, XojPopplerDocument doc) {
	Page * page = pdf->getPage();

	Object * o = new Object();
	page->getContents(o);

	this->resources = page->getResourceDict();

	if (o->getType() == objStream) {
		Dict * dict = o->getStream()->getDict();

		Object filter;
		dict->lookup("Filter", &filter);
		//			// this may would be better, but not working...:-/
		//			Object oDict;
		//			oDict.initDict(dict);
		//			Stream * txtStream = stream->addFilters(oDict);
		//			writePlainStream(txtStream);

		if (filter.isNull()) {
			writePlainStream(o->getStream());
		} else if (filter.isName("FlateDecode")) {
			writeGzStream(o->getStream());
		} else if (filter.isName()) {
			g_warning("Unhandled stream filter: %s\n", filter.getName());
		}
	} else {
		printf("other poppler type: %i\n", o->getType());
	}

	delete o;
	this->resources = NULL;

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

	addPopplerDocument(doc->getPdfDocument());
	currentPdfDoc = doc->getPdfDocument();

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		XojPopplerPage * pdf = doc->getPdfPage(page->getPdfPageNr());
		if (!addPopplerPage(pdf, currentPdfDoc)) {
			return false;
		}
	}

	CairoPdf cPdf;
	cPdf.drawPage(page);
	currentPdfDoc = cPdf.getDocument();
	addPopplerDocument(currentPdfDoc);
	if (!addPopplerPage(cPdf.getPage(), currentPdfDoc)) {
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

bool PdfExport::isWhitespace(int c) {
	return c >= 0 && c <= 0xff && specialChars[c] == 1;
}

guint UpdateRefKey::hashFunction(UpdateRefKey * key) {
	return key->ref.num + key->ref.gen * 13 + key->doc.getId() * 23;
}

bool UpdateRefKey::equalFunction(UpdateRefKey * a, UpdateRefKey * b) {
	return a->ref.gen == b->ref.gen && a->ref.num == b->ref.num && a->doc == b->doc;
}

