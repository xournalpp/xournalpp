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

#include <string>       // for string
#include <string_view>  // for string_view

#include <cairo.h>  // for cairo_surface_t, cairo_status_t

#include "XmlNode.h"  // for XmlNode

class OutputStream;

class XmlImageNode: public XmlNode {
public:
    XmlImageNode(StringUtils::StaticStringView tag);
    virtual ~XmlImageNode();

public:
    void setImage(cairo_surface_t* img);

    void setImageData(std::string_view data);

    static cairo_status_t pngWriteFunction(XmlImageNode* image, const unsigned char* data, unsigned int length);

    void writeOut(OutputStream* out) override;

private:
    cairo_surface_t* img;

    std::string imageData;

    OutputStream* out;
    unsigned int pos;
    unsigned char buffer[30] = {0};
};
