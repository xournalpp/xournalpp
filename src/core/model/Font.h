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
    XojFont() = default;
    XojFont(std::string name, double size);
    XojFont(const XojFont&) = default;
    XojFont(XojFont&&) = default;
    ~XojFont() override = default;

    XojFont& operator=(const XojFont&) = default;
    XojFont& operator=(XojFont&&) = default;

    bool operator==(const XojFont& font) const { return name == font.name && size == font.size; }

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

public:
    const std::string& getName() const;
    void setName(std::string name);

    double getSize() const;
    void setSize(double size);

    /**
     * @return The Pango-style string that represents this
     * font.
     */
    std::string asString() const;

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
