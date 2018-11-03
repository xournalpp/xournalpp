#include "PdfExport.h"

#include "PdfRefEntry.h"
#include "PdfUtil.h"
#include "UpdateRef.h"
#include "UpdateRefKey.h"

#include <config.h>
#include <GzHelper.h>
#include <PageRange.h>

#include <stdlib.h>
#include <string.h>

/**
 * This class uses some inspiration from FPDF (PHP Class)
 */
PdfExport::PdfExport(Document* doc, ProgressListener* progressListener)
{
	XOJ_INIT_TYPE(PdfExport);

	this->doc = doc;
	this->progressListener = progressListener;
	this->xref = new PdfXRef();
	this->writer = new PdfWriter(this->xref);
	this->objectWriter = new PdfObjectWriter(this->writer, this->xref);

	this->dataXrefStart = 0;

	this->pageIds = NULL;

	this->refListsOther = g_hash_table_new_full((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal, g_free,
												(GDestroyNotify) PdfRefList::deletePdfRefList);

	this->resources = NULL;

	this->xref->addXref(0);
	this->xref->addXref(0);

	this->outlineRoot = 0;
}

PdfExport::~PdfExport()
{
	XOJ_CHECK_TYPE(PdfExport);

	for (XojPopplerDocument* doc : this->documents)
	{
		delete doc;
	}
	documents.clear();

	g_list_foreach(this->pageIds, (GFunc) g_free, NULL);
	g_list_free(this->pageIds);
	this->pageIds = NULL;

	this->progressListener = NULL;

	g_hash_table_destroy(this->refListsOther);
	this->refListsOther = NULL;

	delete this->writer;
	this->writer = NULL;

	delete this->objectWriter;
	this->objectWriter = NULL;

	delete this->xref;
	this->xref = NULL;

	XOJ_RELEASE_TYPE(PdfExport);
}

bool PdfExport::writeCatalog()
{
	XOJ_CHECK_TYPE(PdfExport);

	if (!writer->writeObj())
	{
		return false;
	}

	writer->write("<<\n");

	writer->write("/Type /Catalog\n");
	writer->write("/Pages 1 0 R\n");

	writer->write("/OpenAction [3 0 R /FitH null]\n");
//	write("/OpenAction [3 0 R /Fit]\n");

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

	if (this->outlineRoot)
	{
		char* outline = g_strdup_printf("/Outlines %i 0 R\n", this->outlineRoot);
		writer->write(outline);
		g_free(outline);
		writer->write("/PageMode /UseOutlines\n");
	}

	writer->write(">>\nendobj\n");

	return writer->getLastError().empty();
}

bool PdfExport::writeCrossRef()
{
	XOJ_CHECK_TYPE(PdfExport);

	this->dataXrefStart = this->writer->getDataCount();
	this->writer->write("xref\n");
	this->writer->write("0 ");
	char* tmp = g_strdup_printf("%i", this->writer->getObjectId());
	this->writer->write(tmp);
	g_free(tmp);
	this->writer->write("\n");

	this->writer->write("0000000000 65535 f \n");

	char buffer[64];

	for (int i = 0; i < this->xref->getXrefCount(); i++)
	{
		sprintf(buffer, "%010d 00000 n \n", this->xref->getXref(i));
		this->writer->write(buffer);
	}

	return this->writer->getLastError().empty();
}

bool PdfExport::writePagesindex()
{
	XOJ_CHECK_TYPE(PdfExport);

	this->xref->setXref(1, this->writer->getDataCount());
	//Pages root
	this->writer->write("1 0 obj\n");
	this->writer->write("<</Type /Pages\n");
	this->writer->write("/Kids [");

	int pageCount = 0;
	for (GList* l = this->pageIds; l != NULL; l = l->next)
	{
		int id = *((int*) l->data);
		this->writer->write(FORMAT("%i 0 R ", id));
		pageCount++;
	}
	this->writer->write("]\n");
	this->writer->write(FORMAT("/Count %i\n", pageCount));

	PageRef page = doc->getPage(0);

	this->writer->write(FORMAT("/MediaBox [0 0 %.2f %.2f]\n", page->getWidth(), page->getHeight()));
	this->writer->write(">>\n");
	this->writer->write("endobj\n");

	return true;
}

bool PdfExport::writeTrailer()
{
	XOJ_CHECK_TYPE(PdfExport);

	this->writer->write("trailer\n");
	this->writer->write("<<\n");

	this->writer->write(FORMAT("/Size %i\n", this->writer->getObjectId()));
	this->writer->write(FORMAT("/Root %i 0 R\n", (this->writer->getObjectId() - 1)));
	this->writer->write(FORMAT("/Info %i 0 R\n", (this->writer->getObjectId() - 2)));
	this->writer->write(">>\n");
	this->writer->write("startxref\n");

	this->writer->write(FORMAT("%i\n", this->dataXrefStart));
	this->writer->write("%%EOF\n");

	return this->writer->getLastError().empty();
}

void writeOutRef(char* key, PdfRefList* list, gpointer user_data)
{
	list->writeRefList(key);
}

bool PdfExport::writeResourcedict()
{
	XOJ_CHECK_TYPE(PdfExport);

	this->writer->write("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]\n");

	g_hash_table_foreach(this->refListsOther, (GHFunc) writeOutRef, this);

	return true;
}

void writeOutRefObj(char* key, PdfRefList* list, gpointer user_data)
{
	list->writeObjects();
}

bool PdfExport::writeResources()
{
	XOJ_CHECK_TYPE(PdfExport);

	g_hash_table_foreach(this->refListsOther, (GHFunc) writeOutRefObj, this);

	this->objectWriter->writeCopiedObjects();

	//Resource dictionary
	this->xref->setXref(2, this->writer->getDataCount());

	this->writer->write("2 0 obj\n");
	this->writer->write("<<\n");
	if (!writeResourcedict())
	{
		return false;
	}
	this->writer->write(">>\n");
	this->writer->write("endobj\n");

	return true;
}

bool PdfExport::writeFooter()
{
	XOJ_CHECK_TYPE(PdfExport);

	if (!writePagesindex())
	{
		g_warning("failed to write outlines");
		return false;
	}

	if (!writeResources())
	{
		g_warning("failed to write resources");
		return false;
	}

	this->bookmarks.writeOutlines(this->doc, this->writer, &this->outlineRoot, this->pageIds);

	if (!writer->writeInfo(doc->getFilename().string()))
	{
		g_warning("failed to write info");
		return false;
	}

	if (!writeCatalog())
	{
		g_warning("failed to write catalog");
		return false;
	}

	if (!writeCrossRef())
	{
		g_warning("failed to write cross ref");
		return false;
	}

	if (!writeTrailer())
	{
		g_warning("failed to write trailer");
		return false;
	}

	return true;
}

void PdfExport::writeGzStream(Stream* str, GList* replacementList)
{
	XOJ_CHECK_TYPE(PdfExport);

	Object obj1 =  str->getDict()->lookup("Length");
	if (!obj1.isInt())
	{
		g_error("PDFDoc::writeGzStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();

	char* buffer = new char[length];

	str->unfilteredReset();
	for (int i = 0; i < length; i++)
	{
		int c = str->getUnfilteredChar();
		buffer[i] = c;
	}
	string text = GzHelper::gzuncompress(string(buffer, length));
	writeStream(text.c_str(), text.length(), replacementList);

	delete[] buffer;

	str->reset();
}

void PdfExport::writePlainStream(Stream* str, GList* replacementList)
{
	XOJ_CHECK_TYPE(PdfExport);

	Object obj1 = str->getDict()->lookup("Length");
	if (!obj1.isInt())
	{
		g_error("PDFDoc::writePlainStream, no Length in stream dict");
		return;
	}

	const int length = obj1.getInt();

	str->unfilteredReset();

	GString* buffer = g_string_sized_new(10240);

	for (int i = 0; i < length; i++)
	{
		int c = str->getUnfilteredChar();
		buffer = g_string_append_c(buffer, c);
	}

	writeStream(buffer->str, buffer->len, replacementList);

	g_string_free(buffer, true);

	str->reset();
}

void PdfExport::writeStream(const char* str, int len, GList* replacementList)
{
	XOJ_CHECK_TYPE(PdfExport);

	int lastWritten = 0;
	int brackets = 0;

	char lastChar = 0;

	for (int i = 0; i < len; i++)
	{
		char c = str[i];
		if (c == '(' && lastChar != '\\')
		{
			brackets++;
		}
		else if (c == ')' && lastChar != '\\')
		{
			brackets--;
			if (brackets < 0)
			{
				brackets = 0;
			}
		}
		else if (brackets == 0 && str[i] == '/')
		{
			this->writer->write(string(str + lastWritten, i - lastWritten));
			lastWritten = i++;

			char buffer[512];
			int u = i;
			for (; u < len && (u - i) < 512; u++)
			{
				buffer[u - i] = str[u];
				if (PdfUtil::isWhitespace(str[u]))
				{
					buffer[u - i] = 0;
					break;
				}
			}

			for (GList* l = replacementList; l != NULL; l = l->next)
			{
				RefReplacement* f = (RefReplacement*) l->data;
				if (f->name == buffer)
				{
					this->writer->write(FORMAT("/%s%i", f->type, f->newId));
					f->markAsUsed();
					lastWritten = u;

					break;
				}
			}

			//printf("->replacement?: %s\n", buffer);
		}

		lastChar = c;
	}

	this->writer->write(string(str + lastWritten, len - lastWritten));
}

void PdfExport::addPopplerDocument(XojPopplerDocument doc)
{
	XOJ_CHECK_TYPE(PdfExport);

	for (XojPopplerDocument* d : this->documents)
	{
		if (*d == doc)
		{
			return;
		}
	}

	XojPopplerDocument* d = new XojPopplerDocument();
	*d = doc;

	this->documents.push_back(d);
}

bool PdfExport::addPopplerPage(XojPopplerPage* pdf, XojPopplerDocument doc)
{
	XOJ_CHECK_TYPE(PdfExport);

	Page* page = pdf->getPage();
	static int otherObjectId = 1;

	this->resources = page->getResourceDict();

	GList* replacementList = NULL;

	Dict* dict = page->getResourceDict();
	for (int i = 0; i < dict->getLength(); i++)
	{
		const char* cDictName = dict->getKey(i);
		PdfRefList* refList = (PdfRefList*) g_hash_table_lookup(this->refListsOther, cDictName);
		if (!refList)
		{
			char* indexName = NULL;
			if (strcmp(cDictName, "Font") == 0)
			{
				indexName = g_strdup("F");
			}
			else if (strcmp(cDictName, "XObject") == 0)
			{
				indexName = g_strdup("I");
			}
			else if (strcmp(cDictName, "ExtGState") == 0)
			{
				indexName = g_strdup("Gs");
			}
			else if (strcmp(cDictName, "Pattern") == 0)
			{
				indexName = g_strdup("p");
			}
			else
			{
				indexName = g_strdup_printf("o%i-", otherObjectId++);
			}

			refList = new PdfRefList(this->xref, this->objectWriter, this->writer, indexName);
			char* dictName = g_strdup(dict->getKey(i));

			// insert the new RefList into the hash table
			g_hash_table_insert(this->refListsOther, dictName, refList);
		}

		refList->parse(dict, i, doc, replacementList);

	}

	Object* o = new Object(page->getContents());

	if (o->getType() == objStream)
	{
		Dict* dict = o->getStream()->getDict();

		Object filter = dict->lookup("Filter");
//		// this may would be better, but not working...:-/
//		Object oDict;
//		oDict.initDict(dict);
//		Stream * txtStream = stream->addFilters(oDict);
//		writePlainStream(txtStream);

		if (filter.isNull())
		{
			writePlainStream(o->getStream(), replacementList);
		}
		else if (filter.isName("FlateDecode"))
		{
			writeGzStream(o->getStream(), replacementList);
		}
		else if (filter.isName())
		{
			g_warning("Unhandled stream filter: %s\n", filter.getName());
		}
	}
	else if (o->getType() == objArray)
	{
		int arrLength = o->arrayGetLength();
		//g_warning("Array length is %i\n",arrLength); 
		for (int i = 0; i < arrLength; i++)
		{
			Object* subObject = new Object(o->arrayGet(i));
			if (subObject->getType() == objStream)
			{
				Dict* dict = subObject->getStream()->getDict();

				Object filter = dict->lookup("Filter");

				if (filter.isNull())
				{
					writePlainStream(subObject->getStream(), replacementList);
				}
				else if (filter.isName("FlateDecode"))
				{
					writeGzStream(subObject->getStream(), replacementList);
				}
				else if (filter.isName())
				{
					g_warning("Unhandled stream filter: %s\n", filter.getName());
				}
			}
			else
			{
				g_warning("other poppler type: %i\n", subObject->getType());
			}
			delete subObject;
		}
	}
	else
	{
		g_warning("other poppler type: %i\n", o->getType());
	}

	for (GList* l = replacementList; l != NULL; l = l->next)
	{
		RefReplacement* f = (RefReplacement*) l->data;
		delete f;
	}
	g_list_free(replacementList);

	delete o;
	this->resources = NULL;

	return true;
}

bool PdfExport::writePage(int pageNr)
{
	XOJ_CHECK_TYPE(PdfExport);

	PageRef page = doc->getPage(pageNr);

	if (!page)
	{
		return false;
	}

	int* pageId = (int*) g_malloc(sizeof(int));
	*pageId = this->writer->getObjectId();
	this->pageIds = g_list_append(this->pageIds, pageId);

	this->writer->writeObj();
	this->writer->write("<</Type /Page\n");
	this->writer->write("/Parent 1 0 R\n");

	this->writer->write(FORMAT("/MediaBox [0 0 %.2f %.2f]\n", page->getWidth(), page->getHeight()));
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
	this->writer->write(FORMAT("/Contents %i 0 R>>\n", this->writer->getObjectId()));
	this->writer->write("endobj\n");
	//Page content
	this->writer->writeObj();

	this->writer->startStream();

	addPopplerDocument(doc->getPdfDocument());
	currentPdfDoc = doc->getPdfDocument();

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF)
	{
		XojPopplerPage* pdf = doc->getPdfPage(page->getPdfPageNr());
		if (!addPopplerPage(pdf, currentPdfDoc))
		{
			return false;
		}
	}

	currentPdfDoc = cPdf.getDocument();
	if (!addPopplerPage(cPdf.getPage(pageNr), currentPdfDoc))
	{
		return false;
	}

	this->writer->endStream();

	this->writer->write("endobj\n");

	return true;
}

string PdfExport::getLastError()
{
	XOJ_CHECK_TYPE(PdfExport);

	if (this->lastError.empty())
	{
		return this->writer->getLastError();
	}
	return this->lastError;
}

bool PdfExport::createPdf(path file, GList* range)
{
	XOJ_CHECK_TYPE(PdfExport);

	if (range == NULL)
	{
		this->lastError = "No pages to export!";
		return false;
	}

	if (!this->writer->openFile(file))
	{
		return false;
	}
	this->writer->write("%PDF-1.4\n");

	int count = 0;
	for (GList* l = range; l != NULL; l = l->next)
	{
		PageRangeEntry* e = (PageRangeEntry*) l->data;
		count += e->getLast() - e->getFirst();
	}

	if (this->progressListener)
	{
		this->progressListener->setMaximumState(count * 2);
	}

	int c = 0;
	for (GList* l = range; l != NULL; l = l->next)
	{
		PageRangeEntry* e = (PageRangeEntry*) l->data;

		for (int i = e->getFirst(); i < e->getLast(); i++)
		{
			PageRef page = doc->getPage(i);
			cPdf.drawPage(page);
			if (this->progressListener)
			{
				this->progressListener->setCurrentState(c++);
			}
		}
	}

	cPdf.finalize();
	addPopplerDocument(cPdf.getDocument());

	for (int i = 0; i < count; i++)
	{
		if (!writePage(i))
		{
			g_warning("error writing page %i", i + 1);
			return false;
		}

		if (this->progressListener)
		{
			this->progressListener->setCurrentState(i + count);
		}
	}

	// Write our own footer
	if (!writeFooter())
	{
		g_warning("error writing footer");
		return false;
	}

	this->writer->close();

	return true;
}

bool PdfExport::createPdf(path file)
{
	XOJ_CHECK_TYPE(PdfExport);

	if (doc->getPageCount() < 1)
	{
		lastError = "No pages to export!";
		return false;
	}

	if (!this->writer->openFile(file))
	{
		return false;
	}

	this->writer->write("%PDF-1.4\n");

	if (this->progressListener)
	{
		this->progressListener->setMaximumState(doc->getPageCount() * 2);
	}

	int count = doc->getPageCount();

	for (int i = 0; i < count; i++)
	{
		PageRef page = doc->getPage(i);
		cPdf.drawPage(page);

		if (this->progressListener)
		{
			this->progressListener->setCurrentState(i);
		}
	}
	cPdf.finalize();
	addPopplerDocument(cPdf.getDocument());

	for (int i = 0; i < count; i++)
	{
		if (!writePage(i))
		{
			g_warning("error writing page %i", i + 1);
			return false;
		}

		if (this->progressListener)
		{
			this->progressListener->setCurrentState(i + count);
		}
	}

	// Write our own footer
	if (!writeFooter())
	{
		g_warning("error writing footer");
		return false;
	}

	this->writer->close();

	return true;
}
