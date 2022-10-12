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

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <vector>   // for vector

#include "Attribute.h"  // for XMLAttribute

class ProgressListener;
class OutputStream;

class XmlNode {
public:
    XmlNode(const char* tag);
    virtual ~XmlNode() = default;

public:
    void setAttrib(const char* attrib, std::string value);
    void setAttrib(const char* attrib, const char* value);
    void setAttrib(const char* attrib, double value);
    void setAttrib(const char* attrib, int value);
    void setAttrib(const char* attrib, size_t value);

    /**
     * The double array is now owned by XmlNode and automatically deleted!
     */
    void setAttrib(const char* attrib, double* value, int count);

    void writeOut(OutputStream* out, ProgressListener* _listener);

    virtual void writeOut(OutputStream* out) { writeOut(out, nullptr); }

    void addChild(XmlNode* node);

protected:
    void putAttrib(XMLAttribute* a);
    void writeAttributes(OutputStream* out);

protected:
    std::vector<std::unique_ptr<XmlNode>> children{};
    std::vector<std::unique_ptr<XMLAttribute>> attributes{};

    std::string tag;
};
