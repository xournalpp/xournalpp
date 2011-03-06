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
#include "../model/Document.h"
#include "SaveHandler.h"

#include "xml/XmlNode.h"
#include "xml/XmlTextNode.h"
#include "xml/XmlImageNode.h"
#include "xml/XmlPointNode.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include <config.h>

SaveHandler::SaveHandler() {
	root = NULL;
	firstPdfPageVisited = false;
	attachBgId = 1;
	this->backgroundImages = NULL;
}

SaveHandler::~SaveHandler() {
	delete root;

	for (GList * l = this->backgroundImages; l != NULL; l = l->next) {
		delete (BackgroundImage *) l->data;
	}
	g_list_free(this->backgroundImages);
	this->backgroundImages = NULL;
}

void SaveHandler::prepareSave(Document * doc) {
	if (root) {
		// cleanup old data
		delete root;
		root = NULL;

		for (GList * l = this->backgroundImages; l != NULL; l = l->next) {
			delete (BackgroundImage *) l->data;
		}
		g_list_free(this->backgroundImages);
		this->backgroundImages = NULL;
	}

	this->firstPdfPageVisited = false;
	this->attachBgId = 1;

	root = new XmlNode("xournal");
	root->setAttrib("creator", "Xournal++ " VERSION);
	root->setAttrib("fileversion", "2");

	root->addChild(new XmlTextNode("title", "Xournal document - see http://xournal.sourceforge.net/"));
	cairo_surface_t * preview = doc->getPreview();
	if (preview) {
		XmlImageNode * image = new XmlImageNode("preview");
		image->setImage(preview);
		root->addChild(image);
	}

	for (int i = 0; i < doc->getPageCount(); i++) {
		XojPage * p = doc->getPage(i);
		p->backgroundImage.clearSaveState();
	}

	for (int i = 0; i < doc->getPageCount(); i++) {
		XojPage * p = doc->getPage(i);
		visitPage(root, p, doc, i);
	}
}

String SaveHandler::getColorStr(int c) {
	char * str = g_strdup_printf("#%08x", c << 8 | 0xff);
	String color = str;
	g_free(str);
	return color;
}

String SaveHandler::getSolidBgStr(BackgroundType type) {
	switch (type) {
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

void SaveHandler::visitLayer(XmlNode * page, Layer * l) {
	XmlNode * layer = new XmlNode("layer");
	page->addChild(layer);
	ListIterator<Element *> it = l->elementIterator();
	while (it.hasNext()) {
		Element * e = it.next();

		if (e->getType() == ELEMENT_STROKE) {
			Stroke * s = (Stroke *) e;
			XmlPointNode * stroke = new XmlPointNode("stroke");
			layer->addChild(stroke);

			StrokeTool t = s->getToolType();

			if (t == STROKE_TOOL_PEN) {
				stroke->setAttrib("tool", "pen");
			} else if (t == STROKE_TOOL_ERASER) {
				stroke->setAttrib("tool", "eraser");
			} else if (t == STROKE_TOOL_HIGHLIGHTER) {
				stroke->setAttrib("tool", "highlighter");
			} else {
				g_warning("Unknown stroke tool type: %i", t);
				stroke->setAttrib("tool", "pen");
			}

			stroke->setAttrib("color", getColorStr(s->getColor()).c_str());

			bool hasPresureSensitivity = false;
			int pointCount = s->getPointCount();
			Point * points = new Point[pointCount];
			for (int i = 0; i < pointCount; i++) {
				points[i] = s->getPoint(i);
			}

			stroke->setPoints(points, pointCount);

			if (s->hasPressure()) {
				double * values = new double[pointCount + 1];
				values[0] = s->getWidth();
				for (int i = 0; i < pointCount; i++) {
					values[i + 1] = points[i].z;
				}

				stroke->setAttrib("width", values, pointCount);
			} else {
				stroke->setAttrib("width", s->getWidth());
			}

		} else if (e->getType() == ELEMENT_TEXT) {
			Text * t = (Text *) e;
			XmlTextNode * text = new XmlTextNode("text", t->getText().c_str());
			layer->addChild(text);

			XojFont & f = t->getFont();

			text->setAttrib("font", f.getName().c_str());
			text->setAttrib("size", f.getSize());
			text->setAttrib("x", t->getX());
			text->setAttrib("y", t->getY());
			text->setAttrib("color", getColorStr(t->getColor()).c_str());
		} else if (e->getType() == ELEMENT_IMAGE) {
			Image * i = (Image *) e;
			XmlImageNode * image = new XmlImageNode("image");
			layer->addChild(image);

			image->setImage(i->getImage());

			image->setAttrib("left", i->getX());
			image->setAttrib("top", i->getY());
			image->setAttrib("right", i->getX() + i->getElementWidth());
			image->setAttrib("bottom", i->getY() + i->getElementHeight());
		}
	}
}

void SaveHandler::visitPage(XmlNode * root, XojPage * p, Document * doc, int id) {
	XmlNode * page = new XmlNode("page");
	root->addChild(page);
	page->setAttrib("width", p->getWidth());
	page->setAttrib("height", p->getHeight());

	XmlNode * background = new XmlNode("background");
	page->addChild(background);

	switch (p->getBackgroundType()) {
	case BACKGROUND_TYPE_PDF:

		/**
		 * ATTENTION! The original xournal can only read the XML if the attributes are in the right order!
		 * DO NOT CHANGE THE ORDER OF THE ATTRIBUTES!
		 */

		background->setAttrib("type", "pdf");
		if (!firstPdfPageVisited) {
			firstPdfPageVisited = true;

			if (doc->isAttachPdf()) {
				printf("doc->isAttachPdf()\n");
				background->setAttrib("domain", "attach");
				String filename = doc->getFilename();
				filename += ".";
				filename += "bg.pdf";
				background->setAttrib("filename", filename.c_str());

				GError * error = NULL;
				doc->getPdfDocument().save(filename, &error);

				if (error) {
					if (!this->errorMessage.isEmpty()) {
						this->errorMessage += "\n";
					}

					char * msg = g_strdup_printf(_("Could not write background \"%s\", %s"), filename.c_str(), error->message);
					this->errorMessage += msg;
					g_free(msg);

					g_error_free(error);
				}
			} else {
				background->setAttrib("domain", "absolute");
				String pdfName = doc->getPdfFilename();
				background->setAttrib("filename", pdfName.c_str());
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

		int cloneId = p->backgroundImage.getCloneId();
		if (cloneId != -1) {
			background->setAttrib("domain", "clone");
			char * filename = g_strdup_printf("%i", cloneId);
			background->setAttrib("filename", filename);
			g_free(filename);
		} else if (p->backgroundImage.isAttached() && p->backgroundImage.getPixbuf()) {
			char * filename = g_strdup_printf("bg_%d.png", this->attachBgId++);
			background->setAttrib("domain", "attach");
			background->setAttrib("filename", filename);
			p->backgroundImage.setFilename(filename);

			BackgroundImage * img = new BackgroundImage();
			*img = p->backgroundImage;
			this->backgroundImages = g_list_append(this->backgroundImages, img);

			g_free(filename);
			p->backgroundImage.setCloneId(id);
		} else {
			background->setAttrib("domain", "absolute");
			background->setAttrib("filename", p->backgroundImage.getFilename().c_str());
			p->backgroundImage.setCloneId(id);
		}

		break;
	}

	ListIterator<Layer*> it = p->layerIterator();

	if (!it.hasNext()) { // no layer, but we need to write one layer, else the old Xournal cannot read the file
		XmlNode * layer = new XmlNode("layer");
		page->addChild(layer);
	}

	while (it.hasNext()) {
		visitLayer(page, it.next());
	}
}

void SaveHandler::saveTo(OutputStream * out, String filename) {
	out->write("<?xml version=\"1.0\" standalone=\"no\"?>\n");
	root->writeOut(out);

	for (GList * l = this->backgroundImages; l != NULL; l = l->next) {
		BackgroundImage * img = (BackgroundImage *) l->data;

		char * tmpfn = g_strdup_printf("%s.%s", filename.c_str(), img->getFilename().c_str());
		if (!gdk_pixbuf_save(img->getPixbuf(), tmpfn, "png", NULL, NULL)) {
			char * msg = g_strdup_printf(_("Could not write background \"%s\". Continuing anyway."), tmpfn);

			if (!this->errorMessage.isEmpty()) {
				this->errorMessage += "\n";
			}

			this->errorMessage += msg;
			g_free(msg);
		}
		g_free(tmpfn);
	}
}

String SaveHandler::getErrorMessage() {
	return this->errorMessage;
}

