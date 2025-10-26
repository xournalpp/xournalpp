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

class OutputStream;

class XMLAttribute {
public:
    XMLAttribute(std::u8string name);
    virtual ~XMLAttribute();

public:
    virtual void writeOut(OutputStream* out) = 0;

    std::u8string getName();

private:
    std::u8string name;
};
