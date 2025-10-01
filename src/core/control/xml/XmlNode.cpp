#include "XmlNode.h"

#include <utility>  // for move

#include <glib.h>  // for g_free

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "control/xml/Attribute.h"          // for XMLAttribute
#include "util/OutputStream.h"              // for OutputStream

#include "DoubleArrayAttribute.h"  // for DoubleArrayAttribute
#include "DoubleAttribute.h"       // for DoubleAttribute
#include "IntAttribute.h"          // for IntAttribute
#include "SizeTAttribute.h"        // for SizeTAttribute
#include "TextAttribute.h"         // for TextAttribute
#include <functional>
#include <algorithm>  // per std::find
#include "undo/UndoRedoHandler.h"  // per UndoRedoHandler

XmlNode::XmlNode(const char* tag): tag(tag) {}

void XmlNode::setAttrib(const char* attrib, const char* value) {
    if (value == nullptr) {
        value = "";
    }
    putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, std::string value) {
    putAttrib(new TextAttribute(attrib, std::move(value)));
}

void XmlNode::setAttrib(const char* attrib, double value) { putAttrib(new DoubleAttribute(attrib, value)); }

void XmlNode::setAttrib(const char* attrib, int value) { putAttrib(new IntAttribute(attrib, value)); }

void XmlNode::setAttrib(const char* attrib, size_t value) { putAttrib(new SizeTAttribute(attrib, value)); }

/**
 * The double array is now owned by XmlNode and automatically deleted!
 */
void XmlNode::setAttrib(const char* attrib, std::vector<double> values) {
    putAttrib(new DoubleArrayAttribute(attrib, std::move(values)));
}

void XmlNode::writeOut(OutputStream* out, ProgressListener* listener) {
   
    auto pagesToWrite = UndoRedoHandler::pagesChanged;
   
    out->write("<");
    out->write(tag);

    writeAttributes(out);

    if (children.empty()) {
        out->write("/>\n");
    } else {
        out->write(">\n");

        bool isFilteringPages = (!pagesToWrite.empty() && tag == "xournal");

        if (isFilteringPages) {
            g_message("=== FILTERING PAGES ===");
            g_message("Total children: %zu", children.size());
            g_message("Pages to write (%zu): ", pagesToWrite.size());
            for (const auto& p : pagesToWrite) {
                g_message("  - Page number: %zu", p);
            }
        }

        // Il maxState è buggato perché la barra di caricamento sta sempre al massimo
        // Prende solo le pagine da scrivere in considerazione, non so se è un buon metodo
        if (listener) {
            size_t maxState = isFilteringPages ? pagesToWrite.size() : children.size();
            listener->setMaximumState(maxState);

            g_warning("XmlNode::writeOut: Setting maximum state to %zu", maxState);
        }

        //size_t progressCounter = 1;
        size_t pageNumber = 0;  // Contatore per le VERE pagine (tag == "page")
   
        for (size_t i = 0; i < children.size(); i++) {
            bool shouldWrite = true;
            
            if (isFilteringPages) {
                // Se questo child è una pagina, controlla se va scritta
                if (children[i]->tag == "page") {
                    auto it = std::find(pagesToWrite.begin(), pagesToWrite.end(), pageNumber);
                    if (it == pagesToWrite.end()) {
                        g_message("Skipping page %zu (child index %zu)", pageNumber, i);
                        shouldWrite = false;
                    } else {
                        g_message("Writing page %zu (child index %zu)", pageNumber, i);
                    }
                    pageNumber++;  // Incrementa solo per i tag <page>
                } else {
                    // Non è una pagina (es: <title>), scrivila sempre
                    g_message("Writing non-page element: %s (child index %zu)", children[i]->tag.c_str(), i);
                }
            }
            
            if (shouldWrite) {
                children[i]->writeOut(out);
                
                if (listener) {
                    listener->setCurrentState(pageNumber);
                }
            }
        }

        out->write("</");
        out->write(tag);
        out->write(">\n");
        
        if (isFilteringPages) {
            g_message("=== FINISHED WRITING FILTERED PAGES ===");
            g_message("Total pages found: %zu", pageNumber);
        }
    }
}

void XmlNode::addChild(XmlNode* node) { children.emplace_back(node); }

void XmlNode::putAttrib(XMLAttribute* a) {
    for (auto& attrib: attributes) {
        if (attrib->getName() == a->getName()) {
            attrib.reset(a);
            return;
        }
    }

    attributes.emplace_back(a);
}

void XmlNode::writeAttributes(OutputStream* out) {
    for (auto& attrib: attributes) {
        out->write(" ");
        out->write(attrib->getName());
        out->write("=\"");
        attrib->writeOut(out);
        out->write("\"");
    }
}
