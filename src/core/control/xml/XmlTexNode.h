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
    XmlTexNode(const char* tag, const std::vector<std::byte>&& binaryData);
    virtual ~XmlTexNode();

public:
    void writeOut(OutputStream* out) override;

private:
    /**
     * Binary .PNG or .PDF
     */
    const std::vector<std::byte> binaryData;
};
