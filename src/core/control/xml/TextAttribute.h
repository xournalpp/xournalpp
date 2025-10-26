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

#include "Attribute.h"  // for XMLAttribute

class OutputStream;

class TextAttribute: public XMLAttribute {
public:
    TextAttribute(std::u8string name, std::u8string value);
    ~TextAttribute() override;

public:
    void writeOut(OutputStream* out) override;

private:
    std::u8string value;
};
