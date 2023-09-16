/*
 * Xournal++
 *
 * A font with a name and a size
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "util/serializing/Serializable.h"  // for Serializable

class ObjectInputStream;
class ObjectOutputStream;


class XojFont: public Serializable {
public:
    XojFont();
    XojFont(std::string name, double size);

    /**
     * Set this from a Pango-style font description.
     * E.g.
     *   Serif 12
     * sets this' size to 12 and this font's name to Serif.
     *
     * @param description Pango-style font description.
     */
    explicit XojFont(const char* description);
    XojFont& operator=(const std::string& description);

    ~XojFont() override;

public:
    std::string getName() const;
    void setName(std::string name);

    double getSize() const;
    void setSize(double size);

    /**
     * @return The Pango-style string that represents this
     * font.
     */
    std::string asString() const;

    XojFont& operator=(const XojFont& font);


public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    void updateFontDesc();

private:
    std::string name;
    double size = 0;
};
