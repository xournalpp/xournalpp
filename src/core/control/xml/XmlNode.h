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

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include "util/StringUtils.h"  // for StaticStringView

#include "Attribute.h"  // for XMLAttribute

class ProgressListener;
class OutputStream;

class XmlNode {
public:
    XmlNode(StringUtils::StaticStringView tag);
    virtual ~XmlNode() = default;

public:
    void setAttrib(const char8_t* attrib, const std::string& value);
    void setAttrib(const char8_t* attrib, const char* value);
    void setAttrib(const char8_t* attrib, std::u8string value);
    void setAttrib(const char8_t* attrib, const char8_t* value);
    void setAttrib(const char8_t* attrib, double value);
    void setAttrib(const char8_t* attrib, int value);
    void setAttrib(const char8_t* attrib, size_t value);
    void setAttrib(const char8_t* attrib, std::vector<double> values);

    void writeOut(OutputStream* out, ProgressListener* _listener);

    virtual void writeOut(OutputStream* out) { writeOut(out, nullptr); }

    void addChild(XmlNode* node);

protected:
    void putAttrib(XMLAttribute* a);
    void writeAttributes(OutputStream* out);

protected:
    std::vector<std::unique_ptr<XmlNode>> children{};
    std::vector<std::unique_ptr<XMLAttribute>> attributes{};

    StringUtils::StaticStringView tag;
};
