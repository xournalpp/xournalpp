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

#include "XmlNode.h"

class OutputStream;

class XmlTextNode: public XmlNode {
public:
    XmlTextNode(StringUtils::StaticStringView tag, std::string text);
    explicit XmlTextNode(StringUtils::StaticStringView tag);
    ~XmlTextNode() override = default;

public:
    void setText(std::string text);

    void writeOut(OutputStream* out) override;

private:
    std::string text;
};
