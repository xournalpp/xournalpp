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

#include "Attribute.h"  // for XMLAttribute

class OutputStream;

class IntAttribute: public XMLAttribute {
public:
    IntAttribute(const char* name, int value);
    ~IntAttribute() override;

public:
    void writeOut(OutputStream* out) override;

private:
    int value;
};
