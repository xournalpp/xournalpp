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
    XMLAttribute(std::string name);
    virtual ~XMLAttribute();

public:
    virtual void writeOut(OutputStream* out) = 0;

    std::string getName();

private:
    std::string name;
};
