/*
 * Xournal++
 *
 * Saves a document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include "../model/Stroke.h"
#include "../model/Text.h"
#include "../model/Image.h"
#include "../model/TexImage.h"
#include "../model/Document.h"
#include "../model/Layer.h"
#include "../model/BackgroundImage.h"
#include "SaveHandler.h"
#include "jobs/ProgressListener.h"

#include "xml/XmlNode.h"
#include "xml/XmlTextNode.h"
#include "xml/XmlImageNode.h"
#include "xml/XmlTexNode.h"
#include "xml/XmlPointNode.h"

#include <clocale>

#include <config.h>
#include <glib/gi18n-lib.h>

#include <config.h>

#include <iostream>

using namespace std;

// TODO LOW PRIO: remove all elements which are complete outside the pages
// TODO LOW PRIO: remove 0 width line parts
// TODO NEXT-RELEASE: Save Shapes as shapes instead of lines

SaveHandler::SaveHandler()
{
	XOJ_INIT_TYPE(SaveHandler);

	this->root = NULL;
	this->firstPdfPageVisited = false;
	this->attachBgId = 1;
	this->backgroundImages = NULL;
}

SaveHandler::~SaveHandler()
{
	XOJ_CHECK_TYPE(SaveHandler);

	delete this->root;

	for (GList* l = this->backgroundImages; l != NULL; l = l->next)
	{
		delete (BackgroundImage*) l->data;
	}
	g_list_free(this->backgroundImages);
	this->backgroundImages = NULL;

	XOJ_RELEASE_TYPE(SaveHandler);
}

void SaveHandler::prepareSave(Document* doc)
{
	XOJ_CHECK_TYPE(SaveHandler);

	if (this->root)
	{
		// cleanup old data
		delete this->root;
		this->root = NULL;

		for (GList* l = this->backgroundImages; l != NULL; l = l->next)
		{
			delete (BackgroundImage*) l->data;
		}
		g_list_free(this->backgroundImages);
		this->backgroundImages = NULL;
	}

	this->firstPdfPageVisited = false;
	this->attachBgId = 1;

	this->root = new XmlNode("xournal");
	this->root->setAttrib("creator", "Xournal++ " VERSION);
	this->root->setAttrib("fileversion", "2");

	this->root->addChild(new XmlTextNode("title",
										 "Xournal document - see http://xournal.sourceforge.net/"));
	cairo_surface_t* preview = doc->getPreview();
	if (preview)
	{
		XmlImageNode* image = new XmlImageNode("preview");
		image->setImage(preview);
		this->root->addChild(image);
	}

	for (int i = 0; i < doc->getPageCount(); i++)
	{
		PageRef p = doc->getPage(i);
		p->getBackgroundImage().clearSaveState();
	}

	for (int i = 0; i < doc->getPageCount(); i++)
	{
		PageRef p = doc->getPage(i);
		visitPage(this->root, p, doc, i);
	}
}

string SaveHandler::getColorStr(int c, unsigned char alpha)
{
	//TODO rewrite
	char* str = g_strdup_printf("#%08x", c << 8 | alpha);
	string color = str;
	g_free(str);
	return color;
}

string SaveHandler::getSolidBgStr(BackgroundType type)
{
	switch (type)
	{
	case BACKGROUND_TYPE_NONE:
		return "plain";
	case BACKGROUND_TYPE_LINED:
		return "lined";
	case BACKGROUND_TYPE_RULED:
		return "ruled";
	case BACKGROUND_TYPE_GRAPH:
		return "graph";
	}
	return "plain";
}

void SaveHandler::visitLayer(XmlNode* page, Layer* l)
{
	XOJ_CHECK_TYPE(SaveHandler);

	XmlNode* layer = new XmlNode("layer");
	page->addChild(layer);
	for(Element* e : *l->getElements())
	{
		if (e->getType() == ELEMENT_STROKE)
		{
			Stroke* s = (Stroke*) e;
			XmlPointNode* stroke = new XmlPointNode("stroke");
			layer->addChild(stroke);

			StrokeTool t = s->getToolType();

			unsigned char alpha = 0xff;

			if (t == STROKE_TOOL_PEN)
			{
				stroke->setAttrib("tool", "pen");
			}
			else if (t == STROKE_TOOL_ERASER)
			{
				stroke->setAttrib("tool", "eraser");
			}
			else if (t == STROKE_TOOL_HIGHLIGHTER)
			{
				stroke->setAttrib("tool", "highlighter");
				alpha = 0x7f;
			}
			else
			{
				g_warning("Unknown stroke tool type: %i", t);
				stroke->setAttrib("tool", "pen");
			}

			stroke->setAttrib("color", getColorStr(s->getColor(), alpha).c_str());

			int pointCount = s->getPointCount();

			for (int i = 0; i < pointCount; i++)
			{
				Point p = s->getPoint(i);
				stroke->addPoint(&p);
			}

			if (s->hasPressure())
			{
				double* values = new double[pointCount + 1];
				values[0] = s->getWidth();
				for (int i = 0; i < pointCount; i++)
				{
					values[i + 1] = s->getPoint(i).z;
				}

				stroke->setAttrib("width", values, pointCount);
			}
			else
			{
				stroke->setAttrib("width", s->getWidth());
			}

		}
		else if (e->getType() == ELEMENT_TEXT)
		{
			Text* t = (Text*) e;
			XmlTextNode* text = new XmlTextNode("text", t->getText().c_str());
			layer->addChild(text);

			XojFont& f = t->getFont();

			text->setAttrib("font", f.getName().c_str());
			text->setAttrib("size", f.getSize());
			text->setAttrib("x", t->getX());
			text->setAttrib("y", t->getY());
			text->setAttrib("color", getColorStr(t->getColor()).c_str());
		}
		else if (e->getType() == ELEMENT_IMAGE)
		{
			Image* i = (Image*) e;
			XmlImageNode* image = new XmlImageNode("image");
			layer->addChild(image);

			image->setImage(i->getImage());

			image->setAttrib("left", i->getX());
			image->setAttrib("top", i->getY());
			image->setAttrib("right", i->getX() + i->getElementWidth());
			image->setAttrib("bottom", i->getY() + i->getElementHeight());
		}
		else if (e->getType() == ELEMENT_TEXIMAGE)
		{
			TexImage* i = (TexImage*) e;
			XmlTexNode* image = new XmlTexNode("teximage");
			layer->addChild(image);

			image->setImage(i->getImage());

            image->setAttrib("text", i->getText().c_str());
			image->setAttrib("left", i->getX());
			image->setAttrib("top", i->getY());
			image->setAttrib("right", i->getX() + i->getElementWidth());
			image->setAttrib("bottom", i->getY() + i->getElementHeight());
		}
	}
}

void SaveHandler::visitPage(XmlNode* root, PageRef p, Document* doc, int id)
{
	XOJ_CHECK_TYPE(SaveHandler);

	XmlNode* page = new XmlNode("page");
	root->addChild(page);
	page->setAttrib("width", p->getWidth());
	page->setAttrib("height", p->getHeight());

	XmlNode* background = new XmlNode("background");
	page->addChild(background);

	switch (p->getBackgroundType())
	{
	case BACKGROUND_TYPE_PDF:

		/**
		 * ATTENTION! The original xournal can only read the XML if the attributes are in the right order!
		 * DO NOT CHANGE THE ORDER OF THE ATTRIBUTES!
		 */

		background->setAttrib("type", "pdf");
		if (!firstPdfPageVisited)
		{
			firstPdfPageVisited = true;

			if (doc->isAttachPdf())
			{
				cout << "doc->isAttachPdf()" << endl;
				background->setAttrib("domain", "attach");
				path filename = path(doc->getFilename().string() + ".bg.pdf");
				background->setAttrib("filename", filename.c_str());

				GError* error = NULL;
				doc->getPdfDocument().save(filename, &error);

				if (error)
				{
					if (!this->errorMessage.empty())
					{
						this->errorMessage += "\n";
					}

					this->errorMessage += (bl::format(_("Could not write background \"{1}\", {2}"))
										   % filename.string() % error->message).str();

					g_error_free(error);
				}
			}
			else
			{
				background->setAttrib("domain", "absolute");
				background->setAttrib("filename", doc->getPdfFilename().c_str());
			}
		}
		background->setAttrib("pageno", p->getPdfPageNr() + 1);
		break;
	case BACKGROUND_TYPE_NONE:
	case BACKGROUND_TYPE_LINED:
	case BACKGROUND_TYPE_RULED:
	case BACKGROUND_TYPE_GRAPH:
		background->setAttrib("type", "solid");
		background->setAttrib("color", getColorStr(p->getBackgroundColor()).c_str());
		background->setAttrib("style", getSolidBgStr(p->getBackgroundType()).c_str());
		break;
	case BACKGROUND_TYPE_IMAGE:
		background->setAttrib("type", "pixmap");

		int cloneId = p->getBackgroundImage().getCloneId();
		if (cloneId != -1)
		{
			background->setAttrib("domain", "clone");
			char* filename = g_strdup_printf("%i", cloneId);
			background->setAttrib("filename", filename);
			g_free(filename);
		}
		else if (p->getBackgroundImage().isAttached() &&
				 p->getBackgroundImage().getPixbuf())
		{
			char* filename = g_strdup_printf("bg_%d.png", this->attachBgId++);
			background->setAttrib("domain", "attach");
			background->setAttrib("filename", filename);
			p->getBackgroundImage().setFilename(filename);

			BackgroundImage* img = new BackgroundImage();
			*img = p->getBackgroundImage();
			this->backgroundImages = g_list_append(this->backgroundImages, img);

			g_free(filename);
			p->getBackgroundImage().setCloneId(id);
		}
		else
		{
			background->setAttrib("domain", "absolute");
			background->setAttrib("filename", p->getBackgroundImage().getFilename().c_str());
			p->getBackgroundImage().setCloneId(id);
		}

		break;
	}

	if (p->getLayers()->empty()) // no layer, but we need to write one layer, else the old Xournal cannot read the file
	{
		XmlNode* layer = new XmlNode("layer");
		page->addChild(layer);
	}

	for (Layer* l : *p->getLayers())
	{
		visitLayer(page, l);
	}
}

void SaveHandler::saveTo(OutputStream* out,
						 path filename,
						 ProgressListener* listener)
{
	XOJ_CHECK_TYPE(SaveHandler);

	char* old_locale, *saved_locale;

	old_locale = setlocale(LC_NUMERIC, NULL);
	saved_locale = g_strdup(old_locale);

	setlocale(LC_NUMERIC, "C");

	out->write("<?xml version=\"1.0\" standalone=\"no\"?>\n");
	root->writeOut(out, listener);

	for (GList* l = this->backgroundImages; l != NULL; l = l->next)
	{
		BackgroundImage* img = (BackgroundImage*) l->data;

		string tmpfn = CONCAT(filename.string(), ".", img->getFilename().string());
		if (!gdk_pixbuf_save(img->getPixbuf(), tmpfn.c_str(), "png", NULL, NULL))
		{
			if (!this->errorMessage.empty())
			{
				this->errorMessage += "\n";
			}

			this->errorMessage += (bl::format(_("Could not write background \"{1}\". Continuing anyway."))
								   % tmpfn).str();
		}
	}

	setlocale(LC_NUMERIC, saved_locale);
	g_free(saved_locale);
}

string SaveHandler::getErrorMessage()
{
	XOJ_CHECK_TYPE(SaveHandler);

	return this->errorMessage;
}

