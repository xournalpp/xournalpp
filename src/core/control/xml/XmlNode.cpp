#include "XmlNode.h"

#include <utility>  // for move

#include <glib.h>  // for g_free

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "control/xml/Attribute.h"          // for XMLAttribute
#include "util/OutputStream.h"              // for OutputStream

#include "control/Control.h"
#include "model/Document.h"
#include "model/XojPage.h"
#include "model/PageRef.h"
#include "util/StringUtils.h"
#include "DoubleArrayAttribute.h"  // for DoubleArrayAttribute
#include "DoubleAttribute.h"       // for DoubleAttribute
#include "IntAttribute.h"          // for IntAttribute
#include "SizeTAttribute.h"        // for SizeTAttribute
#include "TextAttribute.h"         // for TextAttribute
#include <functional>
#include <algorithm> 
#include "undo/UndoRedoHandler.h"  

class XojPage;
class StringUtils;

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
   
    if ( !StringUtils::isLegacy )
    {
        
        auto pagesToWrite = UndoRedoHandler::pagesChanged;

        g_warning("Pages to write count: %lu", static_cast<unsigned long>(pagesToWrite.size()));

        // Segmentation fault here

        out->write("<");
        out->write(tag);

        writeAttributes(out);

        if (children.empty()) {
            out->write("/>\n");
        } else {
            out->write(">\n");

            bool isFilteringPages = (!pagesToWrite.empty() && tag == "xournal");

            if (listener) {
                size_t maxState = isFilteringPages ? pagesToWrite.size() : children.size();
            }

            size_t pageNumber = 1; 
            size_t writtenPages = 0; 
    
            for (size_t i = 0; i < children.size(); i++) {
                bool shouldWrite = true;
                
                if (isFilteringPages && children[i]->tag == "page") {
                    
                    shouldWrite = false;
                    
                    Control* control = dynamic_cast<Control*>(listener);

                    auto page = control->getDocument()->getPages().at(pageNumber - 1);
                    
                    std::shared_ptr<XojPage> xojPage = page;

                    std::string pageUID = xojPage.get()->getUID();

                    auto it = std::find(pagesToWrite.begin(), pagesToWrite.end(), pageUID);
                    if (it != pagesToWrite.end()) {
                        shouldWrite = true;
                    }
                    
                    pageNumber++;
                }
  
                if (shouldWrite) {
                    children[i]->writeOut(out);
                    
                    if (listener && isFilteringPages && children[i]->tag == "page") {
                        writtenPages++;
                    }
                }
            }

            out->write("</");
            out->write(tag);
            out->write(">\n");
        }
    }
    else
    {
        out->write("<");
        out->write(tag);
        writeAttributes(out);

        if (children.empty()) {
            out->write("/>\n");
        } else {
            out->write(">\n");

            if (listener) {
                listener->setMaximumState(children.size());
            }

            size_t i = 1;

            for (auto& node: children) {
                node->writeOut(out);
                if (listener) {
                    listener->setCurrentState(i);
                }
                i++;
            }

            out->write("</");
            out->write(tag);
            out->write(">\n");
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
