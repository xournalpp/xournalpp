#include "PdfExport.h"
#include <string.h>
#include <config.h>
#include <stdlib.h>
#include "../util/GzHelper.h"

#include "PdfExtGState.h"
#include "PdfFont.h"
#include "PdfRefEntry.h"
#include "PdfUtil.h"
#include "UpdateRef.h"
#include "UpdateRefKey.h"

/**
 * This class uses some inspiration from FPDF (PHP Class)
 */
PdfExport::PdfExport(Document * doc, ProgressListener * progressListener) {
	this->doc = doc;
	this->progressListener = progressListener;
	this->xref = new PdfXRef();
	this->writer = new PdfWriter(this->xref);
	this->objectWriter = new PdfObjectWriter(this->writer, this->xref);

	this->dataXrefStart = 0;

	this->pageCount = 0;

	this->fonts = NULL;
	this->fontId = 1;

	this->images = new PdfRefList(this->xref, this->objectWriter, this->writer, "I");

	this->extGStateId = 1;
	this->extGState = NULL;

	this->pattern = new PdfRefList(this->xref, this->objectWriter, this->writer, "p");

	this->documents = NULL;

	this->resources = NULL;

	this->xref->addXref(0);
	this->xref->addXref(0);

	this->outlineRoot = 0;
}

PdfExport::~PdfExport() {
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

	for (GList * l = this->extGState; l != NULL; l = l->next) {
		PdfExtGState * extGState = (PdfExtGState *) l->data;
		delete extGState;
	}
	g_list_free(this->extGState);

	delete this->pattern;
	this->pattern = NULL;

	delete this->images;
	this->images = NULL;

	this->fonts = NULL;
	this->documents = NULL;
	this->progressListener = NULL;
	this->extGState = NULL;

	delete this->writer;
	this->writer = NULL;

	delete this->objectWriter;
	this->objectWriter = NULL;

	delete this->xref;
	this->xref = NULL;
}

bool PdfExport::writeCatalog() {
	if (!writer->writeObj()) {
		return false;
	}

	writer->write("<<\n");

	writer->write("/Type /Catalog\n");
	writer->write("/Pages 1 0 R\n");

	writer->write("/OpenAction [3 0 R /FitH null]\n");
	//write("/OpenAction [3 0 R /Fit]\n");

	//	if($this->ZoomMode=='fullpage')
	//		$this->_out('/OpenAction [3 0 R /Fit]');
	//	elseif($this->ZoomMode=='fullwidth')
	//		$this->_out('/OpenAction [3 0 R /FitH null]');
	//	elseif($this->ZoomMode=='real')
	//		$this->_out('/OpenAction [3 0 R /XYZ null null 1]');
	//	elseif(!is_string($this->ZoomMode))
	//		$this->_out('/OpenAction [3 0 R /XYZ null null '.($this->ZoomMode/100).']');

	writer->write("/PageLayout /OneColumn\n");

	//	if($this->LayoutMode=='single')
	//		$this->_out('/PageLayout /SinglePage');
	//	elseif($this->LayoutMode=='continuous')
	//		$this->_out('/PageLayout /OneColumn');
	//	elseif($this->LayoutMode=='two')
	//		$this->_out('/PageLayout /TwoColumnLeft');

	if (this->outlineRoot) {
		char * outline = g_strdup_printf("/Outlines %i 0 R\n", this->outlineRoot);
		writer->write(outline);
		g_free(outline);
		writer->write("/PageMode /UseOutlines\n");
	}

	writer->write(">>\nendobj\n");

	return writer->getLastError().isEmpty();
}

bool PdfExport::writeCrossRef() {
	this->dataXrefStart = this->writer->getDataCount();
	this->writer->write("xref\n");
	this->writer->write("0 ");
	char * tmp = g_strdup_printf("%i", this->writer->getObjectId());
	this->writer->write(tmp);
	g_free(tmp);
	this->writer->write("\n");

	this->writer->write("0000000000 65535 f \n");

	char buffer[64];

	for (int i = 0; i < this->xref->getXrefCount(); i++) {
		sprintf(buffer, "%010d 00000 n \n", this->xref->getXref(i));
		this->writer->write(buffer);
	}

	return this->writer->getLastError().isEmpty();
}

bool PdfExport::writePagesindex() {
	this->xref->setXref(1, this->writer->getDataCount());
	//Pages root
	this->writer->write("1 0 obj\n");
	this->writer->write("<</Type /Pages\n");
	this->writer->write("/Kids [");

	for (int i = 0; i < this->pageCount; i++) {
		this->writer->writef("%i 0 R ", 3 + 2 * i);
	}
	this->writer->write("]\n");
	this->writer->writef("/Count %i\n", this->pageCount);

	XojPage * page = doc->getPage(0);

	this->writer->writef("/MediaBox [0 0 %.2F %.2F]\n", page->getWidth(), page->getHeight());
	this->writer->write(">>\n");
	this->writer->write("endobj\n");

}

bool PdfExport::writeTrailer() {
	this->writer->write("trailer\n");
	this->writer->write("<<\n");

	this->writer->writef("/Size %i\n", this->writer->getObjectId());
	this->writer->writef("/Root %i 0 R\n", this->writer->getObjectId() - 1);
	this->writer->writef("/Info %i 0 R\n", this->writer->getObjectId() - 2);
	this->writer->write(">>\n");
	this->writer->write("startxref\n");

	this->writer->writef("%i\n", this->dataXrefStart);
	this->writer->write("%%EOF\n");

	return this->writer->getLastError().isEmpty();
}

bool PdfExport::writeResourcedict() {
	this->writer->write("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]\n");

	if (this->fonts) {
		this->writer->write("/Font <<\n");

		for (GList * l = this->fonts; l != NULL; l = l->next) {
			PdfFont * font = (PdfFont *) l->data;
			this->writer->writef("/F%i %i 0 R\n", font->id, font->objectId);
		}
		this->writer->write(">>\n");
	}

	if (this->images) {
		this->writer->write("/XObject <<\n");
		this->images->writeRefList();
		this->writer->write(">>\n");
	}

	//	if (this->extGState) {
	//		this->writer->write("/XObject <<\n");
	//		for (GList * l = this->extGState; l != NULL; l = l->next) {
	//			PdfExtGState * extGState = (PdfExtGState *) l->data;
	//
	//			// TODO: implementieren
	//			//			this->writer->writef("/GS%i %i 0 R\n", extGState->id, extGState->objectId);
	//		}
	//		this->writer->write(">>\n");
	//	}


	if (this->pattern) {
		this->writer->write("/Pattern <<\n");
		this->pattern->writeRefList();
		this->writer->write(">>\n");
	}

	return true;
}

/**
 * This method write out the font index, also embedded fonts
 */
bool PdfExport::writeFonts() {
	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;

		this->writer->writeObj();
		font->objectId = this->writer->getObjectId() - 1;
		this->objectWriter->writeObject(font->object, font->doc);
		this->writer->write("\nendobj\n");
	}

	return true;
}

bool PdfExport::writeResources() {
	if (!writeFonts()) {
		return false;
	}

	this->images->writeObjects();
	this->pattern->writeObjects();
	this->objectWriter->writeCopiedObjects();

	//Resource dictionary
	this->xref->setXref(2, this->writer->getDataCount());

	this->writer->write("2 0 obj\n");
	this->writer->write("<<\n");
	if (!writeResourcedict()) {
		return false;
	}
	this->writer->write(">>\n");
	this->writer->write("endobj\n");

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

	if (!this->bookmarks.writeOutlines(this->doc, this->writer, &this->outlineRoot)) {
		g_warning("failed to write outlines");
		return false;
	}

	if (!writer->writeInfo(doc->getFilename())) {
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

void PdfExport::writeGzStream(Stream * str, GList * replacementList) {
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
	writeStream(text->str, text->len, replacementList);

	g_string_free(text, true);

	delete buffer;

	str->reset();
}

int PdfExport::lookupFont(String name, Ref ref) {
	for (GList * l = this->fonts; l != NULL; l = l->next) {
		PdfFont * font = (PdfFont *) l->data;
		if (font->doc == currentPdfDoc && font->originalName == name) {
			return font->id;
		}
	}

	Object fonts;

	// TODO: use ref
	this->resources->lookup("Font", &fonts);

	if (!fonts.isDict()) {
		g_warning("PdfExport::Font fonts is not a dictionary!");
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

void PdfExport::writePlainStream(Stream * str, GList * replacementList) {
	Object obj1;
	str->getDict()->lookup("Length", &obj1);
	if (!obj1.isInt()) {
		g_error("PDFDoc::writePlainStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();
	obj1.free();

	str->unfilteredReset();

	GString * buffer = g_string_sized_new(10240);

	for (int i = 0; i < length; i++) {
		int c = str->getUnfilteredChar();
		buffer = g_string_append_c(buffer, c);
	}

	writeStream(buffer->str, buffer->len, replacementList);

	g_string_free(buffer, true);

	str->reset();
}

class RefReplacement {
public:
	RefReplacement(String name, int newId, char type) {
		this->name = name;
		this->newId = newId;
		this->type = type;
	}

	// TODO: add reference counter, remove unused

	String name;
	int newId;
	char type;
};

void PdfExport::writeStream(const char * str, int len, GList * replacementList) {
	int lastWritten = 0;
	int brackets = 0;

	char lastChar = 0;

	for (int i = 0; i < len; i++) {
		char c = str[i];
		if (c == '(' && lastChar != '\\') {
			brackets++;
		} else if (c == ')' && lastChar != '\\') {
			brackets--;
			if (brackets < 0) {
				brackets = 0;
			}
		} else if (brackets == 0 && str[i] == '/') {
			this->writer->writeLen(str + lastWritten, i - lastWritten);
			lastWritten = i++;

			char buffer[512];
			int u = i;
			for (; u < len && (u - i) < 512; u++) {
				buffer[u - i] = str[u];
				if (PdfUtil::isWhitespace(str[u])) {
					buffer[u - i] = 0;
					break;
				}
			}

			for (GList * l = replacementList; l != NULL; l = l->next) {
				RefReplacement * f = (RefReplacement *) l->data;
				if (f->name == buffer) {
					this->writer->writef("/%c%i", f->type, f->newId);
					lastWritten = u;
					break;
				}
			}

			// TODO: debug
			printf("->replacement?: %s\n", buffer);
		}

		lastChar = c;
	}

	this->writer->writeLen(str + lastWritten, len - lastWritten);
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

	this->resources = page->getResourceDict();

	GList * replacementList = NULL;

	Dict * dict = page->getResourceDict();
	for (int i = 0; i < dict->getLength(); i++) {
		if (strcmp(dict->getKey(i), "Font") == 0) {
			Object o;
			dict->getVal(i, &o);

			Dict * fontDict = o.getDict();
			for (int u = 0; u < fontDict->getLength(); u++) {
				Object fontObject;
				fontDict->getValNF(u, &fontObject);
				Ref ref = fontObject.getRef();

				int font = this->lookupFont(fontDict->getKey(u), ref);
				RefReplacement * replacement = new RefReplacement(fontDict->getKey(u), font, 'F');
				replacementList = g_list_append(replacementList, replacement);

				fontObject.free();
			}

			o.free();
		} else if (strcmp(dict->getKey(i), "XObject") == 0) {
			Object o;
			dict->getVal(i, &o);

			Dict * xobjectDict = o.getDict();
			for (int u = 0; u < xobjectDict->getLength(); u++) {
				Object xobjectObject;

				Object * objImage = new Object();
				xobjectDict->getValNF(u, &xobjectObject);
				xobjectDict->getVal(u, objImage);

				int image = this->images->lookup(xobjectDict->getKey(u), xobjectObject.getRef(), objImage, this->currentPdfDoc);
				RefReplacement * replacement = new RefReplacement(xobjectDict->getKey(u), image, 'I');
				replacementList = g_list_append(replacementList, replacement);

				xobjectObject.free();
			}

			o.free();
		} else if (strcmp(dict->getKey(i), "ExtGState") == 0) {
			//TODO: implementieren!!!!!!!!!!

		} else if (strcmp(dict->getKey(i), "Pattern") == 0) {
			Object o;
			dict->getVal(i, &o);

			printf("pattern!\n");

			Dict * patternDict = o.getDict();
			for (int u = 0; u < patternDict->getLength(); u++) {
				Object patternObject;

				printf("pattern entry\n");

				Object * objPattern = new Object();
				patternDict->getValNF(u, &patternObject);
				patternDict->getVal(u, objPattern);

				int patternId = this->pattern->lookup(patternDict->getKey(u), patternObject.getRef(), objPattern, this->currentPdfDoc);
				RefReplacement * replacement = new RefReplacement(patternDict->getKey(u), patternId, 'p');
				replacementList = g_list_append(replacementList, replacement);

				patternObject.free();
			}

			o.free();
		} else {
			printf("dict->%s\n", dict->getKey(i));
		}
	}

	Object * o = new Object();
	page->getContents(o);

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
			writePlainStream(o->getStream(), replacementList);
		} else if (filter.isName("FlateDecode")) {
			writeGzStream(o->getStream(), replacementList);
		} else if (filter.isName()) {
			g_warning("Unhandled stream filter: %s\n", filter.getName());
		}
	} else {
		g_warning("other poppler type: %i\n", o->getType());
	}

	for (GList * l = replacementList; l != NULL; l = l->next) {
		RefReplacement * f = (RefReplacement *) l->data;
		delete f;
	}
	g_list_free(replacementList);

	o->free();
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

	this->writer->writeObj();
	this->writer->write("<</Type /Page\n");
	this->writer->write("/Parent 1 0 R\n");

	this->writer->writef("/MediaBox [0 0 %.2F %.2F]\n", page->getWidth(), page->getHeight());
	this->writer->write("/Resources 2 0 R\n");
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
	this->writer->writef("/Contents %i 0 R>>\n", this->writer->getObjectId());
	this->writer->write("endobj\n");
	//Page content
	this->writer->writeObj();

	this->writer->startStream();

	addPopplerDocument(doc->getPdfDocument());
	currentPdfDoc = doc->getPdfDocument();

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		XojPopplerPage * pdf = doc->getPdfPage(page->getPdfPageNr());
		if (!addPopplerPage(pdf, currentPdfDoc)) {
			return false;
		}
	}

	currentPdfDoc = cPdf.getDocument();
	if (!addPopplerPage(cPdf.getPage(pageNr), currentPdfDoc)) {
		return false;
	}

	this->writer->endStream();

	this->writer->write("endobj\n");

	return true;
}

String PdfExport::getLastError() {
	if (this->lastError.isEmpty()) {
		return this->writer->getLastError();
	}
	return this->lastError;
}

bool PdfExport::createPdf(String uri) {
	if (doc->getPageCount() < 1) {
		lastError = "No pages to export!";
		return false;
	}

	if (!this->writer->openFile(uri.c_str())) {
		return false;
	}

	this->writer->write("%PDF-1.4\n");

	if (this->progressListener) {
		this->progressListener->setMaximumState(doc->getPageCount() * 2);
	}

	int count = doc->getPageCount();

	for (int i = 0; i < count; i++) {
		XojPage * page = doc->getPage(i);
		cPdf.drawPage(page);
	}
	cPdf.finalize();
	addPopplerDocument(cPdf.getDocument());

	for (int i = 0; i < count; i++) {
		if (!writePage(i)) {
			g_warning("error writing page %i", i + 1);
			return false;
		}

		this->progressListener->setCurrentState(i + count);
	}

	// Write our own footer
	if (!writeFooter()) {
		g_warning("error writing footer");
		return false;
	}

	this->writer->close();

	return true;
}


/**
 *
 * <?php
require('fpdf.php');

class PDF_Rotate extends FPDF
{
var $angle=0;

function Rotate($angle, $x=-1, $y=-1)
{
    if($x==-1)
        $x=$this->x;
    if($y==-1)
        $y=$this->y;
    if($this->angle!=0)
        $this->_out('Q');
    $this->angle=$angle;
    if($angle!=0)
    {
        $angle*=M_PI/180;
        $c=cos($angle);
        $s=sin($angle);
        $cx=$x*$this->k;
        $cy=($this->h-$y)*$this->k;
        $this->_out(sprintf('q %.5f %.5f %.5f %.5f %.2f %.2f cm 1 0 0 1 %.2f %.2f cm', $c, $s, -$s, $c, $cx, $cy, -$cx, -$cy));
    }
}

function _endpage()
{
    if($this->angle!=0)
    {
        $this->angle=0;
        $this->_out('Q');
    }
    parent::_endpage();
}
}
?>
 */

