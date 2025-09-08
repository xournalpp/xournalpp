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

#include "XmlAudioNode.h"  // for XmlAudioNode

class OutputStream;

class XmlTextNode: public XmlAudioNode {
public:
    XmlTextNode(const std::string_view tag, std::string text);
    explicit XmlTextNode(const std::string_view tag);
    ~XmlTextNode() override = default;

public:
    void setText(std::string text);

    void writeOut(OutputStream* out) override;

private:
    std::string text;
};
