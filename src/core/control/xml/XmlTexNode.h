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

#include <vector>  // for vector

#include "XmlNode.h"  // for XmlNode

class OutputStream;

class XmlTexNode: public XmlNode {
public:
    XmlTexNode(const char* tag, std::vector<std::byte> const& binaryData);
    virtual ~XmlTexNode();

public:
    void writeOut(OutputStream* out) override;

private:
    /**
     * Binary .PNG or .PDF
     */
    std::vector<std::byte> const& binaryData;
};
