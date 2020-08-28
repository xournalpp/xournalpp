#include "SaveHandler.h"

#include <config.h>

#include "control/jobs/ProgressListener.h"
#include "control/pagetype/PageTypeHandler.h"
#include "control/xml/XmlImageNode.h"
#include "control/xml/XmlNode.h"
#include "control/xml/XmlPointNode.h"
#include "control/xml/XmlTexNode.h"
#include "control/xml/XmlTextNode.h"
#include "model/BackgroundImage.h"
#include "model/Document.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/StrokeStyle.h"
#include "model/TexImage.h"
#include "model/Text.h"

#include "PathUtil.h"
#include "i18n.h"

SaveHandler::SaveHandler() {
    this->root = nullptr;
    this->firstPdfPageVisited = false;
    this->attachBgId = 1;
    this->backgroundImages = nullptr;
}

SaveHandler::~SaveHandler() {
    delete this->root;

    for (GList* l = this->backgroundImages; l != nullptr; l = l->next) {
        delete static_cast<BackgroundImage*>(l->data);
    }
    g_list_free(this->backgroundImages);
    this->backgroundImages = nullptr;
}

void SaveHandler::prepareSave(Document* doc) {
    if (this->root) {
        // cleanup old data
        delete this->root;
        this->root = nullptr;

        for (GList* l = this->backgroundImages; l != nullptr; l = l->next) {
            delete static_cast<BackgroundImage*>(l->data);
        }
        g_list_free(this->backgroundImages);
        this->backgroundImages = nullptr;
    }

    this->firstPdfPageVisited = false;
    this->attachBgId = 1;

    this->root = new XmlNode("xournal");

    writeHeader();

    cairo_surface_t* preview = doc->getPreview();
    if (preview) {
        auto* image = new XmlImageNode("preview");
        image->setImage(preview);
        this->root->addChild(image);
    }

    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);
        p->getBackgroundImage().clearSaveState();
    }

    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);
        visitPage(this->root, p, doc, i);
    }
}

void SaveHandler::writeHeader() {
    this->root->setAttrib("creator", PROJECT_STRING);
    this->root->setAttrib("fileversion", FILE_FORMAT_VERSION);
    this->root->addChild(new XmlTextNode("title", std::string{"Xournal++ document - see "} + PROJECT_URL));
}

auto SaveHandler::getColorStr(Color c, unsigned char alpha) -> string {
    char* str = g_strdup_printf("#%08x", uint32_t(c) << 8U | alpha);
    string color = str;
    g_free(str);
    return color;
}

void SaveHandler::writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode) {
    /** set stroke timestamp value to the XmlPointNode */
    xmlAudioNode->setAttrib("ts", audioElement->getTimestamp());
    xmlAudioNode->setAttrib("fn", audioElement->getAudioFilename());
}

void SaveHandler::visitStroke(XmlPointNode* stroke, Stroke* s) {
    StrokeTool t = s->getToolType();

    unsigned char alpha = 0xff;

    if (t == STROKE_TOOL_PEN) {
        stroke->setAttrib("tool", "pen");
        writeTimestamp(s, stroke);
    } else if (t == STROKE_TOOL_ERASER) {
        stroke->setAttrib("tool", "eraser");
    } else if (t == STROKE_TOOL_HIGHLIGHTER) {
        stroke->setAttrib("tool", "highlighter");
        alpha = 0x7f;
    } else {
        g_warning("Unknown stroke tool type: %i", t);
        stroke->setAttrib("tool", "pen");
    }

    stroke->setAttrib("color", getColorStr(s->getColor(), alpha).c_str());

    int pointCount = s->getPointCount();

    for (int i = 0; i < pointCount; i++) {
        Point p = s->getPoint(i);
        stroke->addPoint(&p);
    }

    if (s->hasPressure()) {
        auto* values = new double[pointCount + 1];
        values[0] = s->getWidth();
        for (int i = 0; i < pointCount; i++) {
            values[i + 1] = s->getPoint(i).z;
        }

        stroke->setAttrib("width", values, pointCount);
    } else {
        stroke->setAttrib("width", s->getWidth());
    }

    visitStrokeExtended(stroke, s);
}

/**
 * Export the fill attributes
 */
void SaveHandler::visitStrokeExtended(XmlPointNode* stroke, Stroke* s) {
    if (s->getFill() != -1) {
        stroke->setAttrib("fill", s->getFill());
    }

    if (s->getLineStyle().hasDashes()) {
        stroke->setAttrib("style", StrokeStyle::formatStyle(s->getLineStyle()));
    }
}

void SaveHandler::visitLayer(XmlNode* page, Layer* l) {
    auto* layer = new XmlNode("layer");
    page->addChild(layer);
    for (Element* e: *l->getElements()) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e);
            auto* stroke = new XmlPointNode("stroke");
            layer->addChild(stroke);
            visitStroke(stroke, s);
        } else if (e->getType() == ELEMENT_TEXT) {
            Text* t = dynamic_cast<Text*>(e);
            auto* text = new XmlTextNode("text", t->getText());
            layer->addChild(text);

            XojFont& f = t->getFont();

            text->setAttrib("font", f.getName().c_str());
            text->setAttrib("size", f.getSize());
            text->setAttrib("x", t->getX());
            text->setAttrib("y", t->getY());
            text->setAttrib("color", getColorStr(t->getColor()).c_str());

            writeTimestamp(t, text);
        } else if (e->getType() == ELEMENT_IMAGE) {
            auto* i = dynamic_cast<Image*>(e);
            auto* image = new XmlImageNode("image");
            layer->addChild(image);

            image->setImage(i->getImage());

            image->setAttrib("left", i->getX());
            image->setAttrib("top", i->getY());
            image->setAttrib("right", i->getX() + i->getElementWidth());
            image->setAttrib("bottom", i->getY() + i->getElementHeight());
        } else if (e->getType() == ELEMENT_TEXIMAGE) {
            auto* i = dynamic_cast<TexImage*>(e);
            auto* image = new XmlTexNode("teximage", std::string(i->getBinaryData()));
            layer->addChild(image);

            image->setAttrib("text", i->getText().c_str());
            image->setAttrib("left", i->getX());
            image->setAttrib("top", i->getY());
            image->setAttrib("right", i->getX() + i->getElementWidth());
            image->setAttrib("bottom", i->getY() + i->getElementHeight());
        }
    }
}

void SaveHandler::visitPage(XmlNode* root, PageRef p, Document* doc, int id) {
    auto* page = new XmlNode("page");
    root->addChild(page);
    page->setAttrib("width", p->getWidth());
    page->setAttrib("height", p->getHeight());

    auto* background = new XmlNode("background");
    page->addChild(background);

    if (p->getBackgroundType().isPdfPage()) {
        /**
         * ATTENTION! The original xournal can only read the XML if the attributes are in the right order!
         * DO NOT CHANGE THE ORDER OF THE ATTRIBUTES!
         */

        background->setAttrib("type", "pdf");
        if (!firstPdfPageVisited) {
            firstPdfPageVisited = true;

            if (doc->isAttachPdf()) {
                background->setAttrib("domain", "attach");
                auto filepath = doc->getFilepath();
                Util::clearExtensions(filepath);
                filepath += ".xopp.bg.pdf";
                background->setAttrib("filename", "bg.pdf");

                GError* error = nullptr;
                doc->getPdfDocument().save(filepath, &error);

                if (error) {
                    if (!this->errorMessage.empty()) {
                        this->errorMessage += "\n";
                    }
                    this->errorMessage +=
                            FS(_F("Could not write background \"{1}\", {2}") % filepath.string() % error->message);

                    g_error_free(error);
                }
            } else {
                background->setAttrib("domain", "absolute");
                background->setAttrib("filename", doc->getPdfFilepath().string());
            }
        }
        background->setAttrib("pageno", p->getPdfPageNr() + 1);
    } else if (p->getBackgroundType().isImagePage()) {
        background->setAttrib("type", "pixmap");

        int cloneId = p->getBackgroundImage().getCloneId();
        if (cloneId != -1) {
            background->setAttrib("domain", "clone");
            char* filename = g_strdup_printf("%i", cloneId);
            background->setAttrib("filename", filename);
            g_free(filename);
        } else if (p->getBackgroundImage().isAttached() && p->getBackgroundImage().getPixbuf()) {
            char* filename = g_strdup_printf("bg_%d.png", this->attachBgId++);
            background->setAttrib("domain", "attach");
            background->setAttrib("filename", filename);
            p->getBackgroundImage().setFilepath(filename);

            auto* img = new BackgroundImage();
            *img = p->getBackgroundImage();
            this->backgroundImages = g_list_append(this->backgroundImages, img);

            g_free(filename);
            p->getBackgroundImage().setCloneId(id);
        } else {
            background->setAttrib("domain", "absolute");
            background->setAttrib("filename", p->getBackgroundImage().getFilepath().string());
            p->getBackgroundImage().setCloneId(id);
        }
    } else {
        writeSolidBackground(background, p);
    }

    // no layer, but we need to write one layer, else the old Xournal cannot read the file
    if (p->getLayers()->empty()) {
        auto* layer = new XmlNode("layer");
        page->addChild(layer);
    }

    for (Layer* l: *p->getLayers()) {
        visitLayer(page, l);
    }
}

void SaveHandler::writeSolidBackground(XmlNode* background, PageRef p) {
    background->setAttrib("type", "solid");
    background->setAttrib("color", getColorStr(p->getBackgroundColor()));

    background->setAttrib("style", PageTypeHandler::getStringForPageTypeFormat(p->getBackgroundType().format));

    // Not compatible with Xournal, so the background needs
    // to be changed to a basic one!
    if (!p->getBackgroundType().config.empty()) {
        background->setAttrib("config", p->getBackgroundType().config);
    }
}

void SaveHandler::saveTo(const fs::path& filepath, ProgressListener* listener) {
    GzOutputStream out(filepath);

    if (!out.getLastError().empty()) {
        this->errorMessage = out.getLastError();
        return;
    }

    saveTo(&out, filepath, listener);

    out.close();

    if (this->errorMessage.empty()) {
        this->errorMessage = out.getLastError();
    }
}

void SaveHandler::saveTo(OutputStream* out, const fs::path& filepath, ProgressListener* listener) {
    // XMLNode should be locale-safe ( store doubles using Locale 'C' format

    out->write("<?xml version=\"1.0\" standalone=\"no\"?>\n");
    root->writeOut(out, listener);

    for (GList* l = this->backgroundImages; l != nullptr; l = l->next) {
        auto* img = static_cast<BackgroundImage*>(l->data);

        auto tmpfn = (fs::path(filepath) += ".") += img->getFilepath();
        if (!gdk_pixbuf_save(img->getPixbuf(), tmpfn.u8string().c_str(), "png", nullptr, nullptr)) {
            if (!this->errorMessage.empty()) {
                this->errorMessage += "\n";
            }

            this->errorMessage += FS(_F("Could not write background \"{1}\". Continuing anyway.") % tmpfn.string());
        }
    }
}

auto SaveHandler::getErrorMessage() -> string { return this->errorMessage; }
