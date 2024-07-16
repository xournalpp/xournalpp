#include "SaveHandler.h"

#include <cinttypes>   // for PRIx32
#include <cstdint>     // for uint32_t
#include <cstdio>      // for sprintf, size_t
#include <filesystem>  // for exists

#include <cairo.h>                  // for cairo_surface_t
#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_save
#include <glib.h>                   // for g_free, g_strdup_printf

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeHandler
#include "control/xml/XmlAudioNode.h"          // for XmlAudioNode
#include "control/xml/XmlImageNode.h"          // for XmlImageNode
#include "control/xml/XmlNode.h"               // for XmlNode
#include "control/xml/XmlPointNode.h"          // for XmlPointNode
#include "control/xml/XmlTexNode.h"            // for XmlTexNode
#include "control/xml/XmlTextNode.h"           // for XmlTextNode
#include "control/xojfile/XmlAttrs.h"          // for XmlAttrs
#include "control/xojfile/XmlParserHelper.h"   // for Domain, DOMAIN_NAMES
#include "control/xojfile/XmlTags.h"           // for XmlTags
#include "model/AudioElement.h"                // for AudioElement
#include "model/BackgroundImage.h"             // for BackgroundImage
#include "model/Document.h"                    // for Document
#include "model/Element.h"                     // for Element, ELEMENT_IMAGE
#include "model/Font.h"                        // for XojFont
#include "model/Image.h"                       // for Image
#include "model/Layer.h"                       // for Layer
#include "model/LineStyle.h"                   // for LineStyle
#include "model/PageType.h"                    // for PageType
#include "model/Point.h"                       // for Point
#include "model/Stroke.h"                      // for Stroke, StrokeCapStyle
#include "model/StrokeStyle.h"                 // for StrokeStyle
#include "model/TexImage.h"                    // for TexImage
#include "model/Text.h"                        // for Text
#include "model/XojPage.h"                     // for XojPage
#include "pdf/base/XojPdfDocument.h"           // for XojPdfDocument
#include "util/OutputStream.h"                 // for GzOutputStream, Output...
#include "util/PathUtil.h"                     // for clearExtensions
#include "util/PlaceholderString.h"            // for PlaceholderString
#include "util/i18n.h"                         // for FS, _F

#include "config.h"  // for FILE_FORMAT_VERSION


static constexpr auto& TAG_NAMES = xoj::xml_tags::NAMES;
using TagType = xoj::xml_tags::Type;

SaveHandler::SaveHandler() {
    this->firstPdfPageVisited = false;
    this->attachBgId = 1;
}

void SaveHandler::prepareSave(Document* doc) {
    if (this->root) {
        // cleanup old data
        backgroundImages.clear();
    }

    this->firstPdfPageVisited = false;
    this->attachBgId = 1;

    root.reset(new XmlNode(TAG_NAMES[TagType::XOURNAL]));

    writeHeader();

    cairo_surface_t* preview = doc->getPreview();
    if (preview) {
        auto* image = new XmlImageNode(TAG_NAMES[TagType::PREVIEW]);
        image->setImage(preview);
        this->root->addChild(image);
    }

    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);
        p->getBackgroundImage().clearSaveState();
    }

    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);
        visitPage(root.get(), p, doc, static_cast<int>(i));
    }
}

void SaveHandler::writeHeader() {
    this->root->setAttrib(xoj::xml_attrs::CREATOR_STR, PROJECT_STRING);
    this->root->setAttrib(xoj::xml_attrs::FILEVERSION_STR, FILE_FORMAT_VERSION);
    this->root->addChild(new XmlTextNode(TAG_NAMES[TagType::TITLE],
                                         std::string{"Xournal++ document - see "} + PROJECT_HOMEPAGE_URL));
}

auto SaveHandler::getColorStr(Color c, unsigned char alpha) -> std::string {
    char str[10];
    sprintf(str, "#%08" PRIx32, uint32_t(c) << 8U | alpha);
    std::string color(str);
    return color;
}

void SaveHandler::writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode) {
    if (!audioElement->getAudioFilename().empty()) {
        /** set stroke timestamp value to the XmlPointNode */
        xmlAudioNode->setAttrib(xoj::xml_attrs::TIMESTAMP_STR, audioElement->getTimestamp());
        xmlAudioNode->setAttrib(xoj::xml_attrs::AUDIO_FILENAME_STR, audioElement->getAudioFilename().u8string());
    }
}

void SaveHandler::visitStroke(XmlPointNode* stroke, Stroke* s) {
    StrokeTool t = s->getToolType();

    unsigned char alpha = 0xff;

    if (t >= StrokeTool::NAMES.size()) {
        g_warning("Unknown StrokeTool::Value: %d", static_cast<unsigned int>(t));
        t = StrokeTool::PEN;
    }

    stroke->setAttrib(xoj::xml_attrs::TOOL_STR, StrokeTool::NAMES[t]);

    if (t == StrokeTool::PEN) {
        writeTimestamp(s, stroke);
    } else if (t == StrokeTool::HIGHLIGHTER) {
        alpha = 0x7f;
    }

    stroke->setAttrib(xoj::xml_attrs::COLOR_STR, getColorStr(s->getColor(), alpha).c_str());

    const auto& pts = s->getPointVector();

    stroke->setPoints(pts);

    if (s->hasPressure()) {
        std::vector<double> values;
        values.reserve(pts.size() + 1);
        values.emplace_back(s->getWidth());
        std::transform(pts.begin(), pts.end() - 1, std::back_inserter(values), [](const Point& p) { return p.z; });
        stroke->setAttrib(xoj::xml_attrs::WIDTH_STR, std::move(values));
    } else {
        stroke->setAttrib(xoj::xml_attrs::WIDTH_STR, s->getWidth());
    }

    visitStrokeExtended(stroke, s);
}

/**
 * Export the fill attributes
 */
void SaveHandler::visitStrokeExtended(XmlPointNode* stroke, Stroke* s) {
    if (s->getFill() != -1) {
        stroke->setAttrib(xoj::xml_attrs::FILL_STR, s->getFill());
    }

    StrokeCapStyle capStyle = s->getStrokeCapStyle();

    if (capStyle >= STROKE_CAP_STYLE_NAMES.size()) {
        g_warning("Unknown stroke cap type: %d", capStyle);
        capStyle = StrokeCapStyle::ROUND;
    }

    stroke->setAttrib(xoj::xml_attrs::CAPSTYLE_STR, STROKE_CAP_STYLE_NAMES[capStyle]);

    if (s->getLineStyle().hasDashes()) {
        stroke->setAttrib(xoj::xml_attrs::STYLE_STR, StrokeStyle::formatStyle(s->getLineStyle()));
    }
}

void SaveHandler::visitLayer(XmlNode* page, Layer* l) {
    auto* layer = new XmlNode(TAG_NAMES[TagType::LAYER]);
    page->addChild(layer);
    if (l->hasName()) {
        layer->setAttrib(xoj::xml_attrs::NAME_STR, l->getName().c_str());
    }

    for (auto&& e: l->getElements()) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e.get());
            auto* stroke = new XmlPointNode(TAG_NAMES[TagType::STROKE]);
            layer->addChild(stroke);
            visitStroke(stroke, s);
        } else if (e->getType() == ELEMENT_TEXT) {
            Text* t = dynamic_cast<Text*>(e.get());
            auto* text = new XmlTextNode(TAG_NAMES[TagType::TEXT], t->getText());
            layer->addChild(text);

            XojFont& f = t->getFont();

            text->setAttrib(xoj::xml_attrs::FONT_STR, f.getName().c_str());
            text->setAttrib(xoj::xml_attrs::SIZE_STR, f.getSize());
            text->setAttrib(xoj::xml_attrs::X_COORD_STR, t->getX());
            text->setAttrib(xoj::xml_attrs::Y_COORD_STR, t->getY());
            text->setAttrib(xoj::xml_attrs::COLOR_STR, getColorStr(t->getColor()).c_str());

            writeTimestamp(t, text);
        } else if (e->getType() == ELEMENT_IMAGE) {
            auto* i = dynamic_cast<Image*>(e.get());
            auto* image = new XmlImageNode(TAG_NAMES[TagType::IMAGE]);
            layer->addChild(image);

            image->setImage(i->getImage());

            image->setAttrib(xoj::xml_attrs::LEFT_POS_STR, i->getX());
            image->setAttrib(xoj::xml_attrs::TOP_POS_STR, i->getY());
            image->setAttrib(xoj::xml_attrs::RIGHT_POS_STR, i->getX() + i->getElementWidth());
            image->setAttrib(xoj::xml_attrs::BOTTOM_POS_STR, i->getY() + i->getElementHeight());
        } else if (e->getType() == ELEMENT_TEXIMAGE) {
            auto* i = dynamic_cast<TexImage*>(e.get());
            auto* image = new XmlTexNode(TAG_NAMES[TagType::TEXIMAGE], std::string(i->getBinaryData()));
            layer->addChild(image);

            image->setAttrib(xoj::xml_attrs::TEXT_STR, i->getText().c_str());
            image->setAttrib(xoj::xml_attrs::LEFT_POS_STR, i->getX());
            image->setAttrib(xoj::xml_attrs::TOP_POS_STR, i->getY());
            image->setAttrib(xoj::xml_attrs::RIGHT_POS_STR, i->getX() + i->getElementWidth());
            image->setAttrib(xoj::xml_attrs::BOTTOM_POS_STR, i->getY() + i->getElementHeight());
        }
    }
}

void SaveHandler::visitPage(XmlNode* root, PageRef p, Document* doc, int id) {
    auto* page = new XmlNode(TAG_NAMES[TagType::PAGE]);
    root->addChild(page);
    page->setAttrib(xoj::xml_attrs::WIDTH_STR, p->getWidth());
    page->setAttrib(xoj::xml_attrs::HEIGHT_STR, p->getHeight());

    auto* background = new XmlNode(TAG_NAMES[TagType::BACKGROUND]);
    page->addChild(background);

    writeBackgroundName(background, p);

    using namespace XmlParserHelper;  // for Domain and DOMAIN_NAMES

    if (p->getBackgroundType().isPdfPage()) {
        /**
         * ATTENTION! The original xournal can only read the XML if the attributes are in the right order!
         * DO NOT CHANGE THE ORDER OF THE ATTRIBUTES!
         */

        background->setAttrib(xoj::xml_attrs::TYPE_STR, "pdf");
        if (!firstPdfPageVisited) {
            firstPdfPageVisited = true;

            if (doc->isAttachPdf()) {
                background->setAttrib(xoj::xml_attrs::DOMAIN_STR, DOMAIN_NAMES[Domain::ATTACH]);
                auto filepath = doc->getFilepath();
                Util::clearExtensions(filepath);
                filepath += ".xopp.bg.pdf";
                background->setAttrib(xoj::xml_attrs::FILENAME_STR, "bg.pdf");

                GError* error = nullptr;
                if (!exists(filepath)) {
                    doc->getPdfDocument().save(filepath, &error);
                }

                if (error) {
                    if (!this->errorMessage.empty()) {
                        this->errorMessage += "\n";
                    }
                    this->errorMessage +=
                            FS(_F("Could not write background \"{1}\", {2}") % filepath.u8string() % error->message);

                    g_error_free(error);
                }
            } else {
                background->setAttrib(xoj::xml_attrs::DOMAIN_STR, DOMAIN_NAMES[Domain::ABSOLUTE]);
                background->setAttrib(xoj::xml_attrs::FILENAME_STR, doc->getPdfFilepath().u8string());
            }
        }
        background->setAttrib(xoj::xml_attrs::PAGE_NUMBER_STR, p->getPdfPageNr() + 1);
    } else if (p->getBackgroundType().isImagePage()) {
        background->setAttrib(xoj::xml_attrs::TYPE_STR, "pixmap");

        int cloneId = p->getBackgroundImage().getCloneId();
        if (cloneId != -1) {
            background->setAttrib(xoj::xml_attrs::DOMAIN_STR, DOMAIN_NAMES[Domain::CLONE]);
            char* filename = g_strdup_printf("%i", cloneId);
            background->setAttrib(xoj::xml_attrs::FILENAME_STR, filename);
            g_free(filename);
        } else if (p->getBackgroundImage().isAttached() && p->getBackgroundImage().getPixbuf()) {
            char* filename = g_strdup_printf("bg_%d.png", this->attachBgId++);
            background->setAttrib(xoj::xml_attrs::DOMAIN_STR, DOMAIN_NAMES[Domain::ATTACH]);
            background->setAttrib(xoj::xml_attrs::FILENAME_STR, filename);
            p->getBackgroundImage().setFilepath(filename);

            backgroundImages.emplace_back(p->getBackgroundImage());

            g_free(filename);
            p->getBackgroundImage().setCloneId(id);
        } else {
            background->setAttrib(xoj::xml_attrs::DOMAIN_STR, DOMAIN_NAMES[Domain::ABSOLUTE]);
            background->setAttrib(xoj::xml_attrs::FILENAME_STR, p->getBackgroundImage().getFilepath().u8string());
            p->getBackgroundImage().setCloneId(id);
        }
    } else {
        writeSolidBackground(background, p);
    }

    // no layer, but we need to write one layer, else the old Xournal cannot read the file
    if (p->getLayers()->empty()) {
        auto* layer = new XmlNode(TAG_NAMES[TagType::LAYER]);
        page->addChild(layer);
    }

    for (Layer* l: *p->getLayers()) {
        visitLayer(page, l);
    }
}

void SaveHandler::writeSolidBackground(XmlNode* background, PageRef p) {
    background->setAttrib(xoj::xml_attrs::TYPE_STR, "solid");
    background->setAttrib(xoj::xml_attrs::COLOR_STR, getColorStr(p->getBackgroundColor()));
    background->setAttrib(xoj::xml_attrs::STYLE_STR,
                          PageTypeHandler::getStringForPageTypeFormat(p->getBackgroundType().format));

    // Not compatible with Xournal, so the background needs
    // to be changed to a basic one!
    if (!p->getBackgroundType().config.empty()) {
        background->setAttrib(xoj::xml_attrs::CONFIG_STR, p->getBackgroundType().config);
    }
}

void SaveHandler::writeBackgroundName(XmlNode* background, PageRef p) {
    if (p->backgroundHasName()) {
        background->setAttrib(xoj::xml_attrs::NAME_STR, p->getBackgroundName());
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

    for (BackgroundImage const& img: backgroundImages) {
        auto tmpfn = (fs::path(filepath) += ".") += img.getFilepath();
        if (!gdk_pixbuf_save(img.getPixbuf(), tmpfn.u8string().c_str(), "png", nullptr, nullptr)) {
            if (!this->errorMessage.empty()) {
                this->errorMessage += "\n";
            }

            this->errorMessage += FS(_F("Could not write background \"{1}\". Continuing anyway.") % tmpfn.u8string());
        }
    }
}

auto SaveHandler::getErrorMessage() -> std::string { return this->errorMessage; }
