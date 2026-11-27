/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <cairo.h>  // for cairo_surface_t

#include "XmlNode.h"  // for XmlNode

class OutputStream;

class XmlImageNode: public XmlNode {
public:
    XmlImageNode(StringUtils::StaticStringView tag);
    virtual ~XmlImageNode();

public:
    void setImage(cairo_surface_t* img);

    void setImageData(std::string data);

    void writeOut(OutputStream* out) override;

private:
    cairo_surface_t* img = nullptr;

    std::string imageData;
};
