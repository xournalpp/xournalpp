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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "util/serializing/Serializable.h"


class XojFont: public Serializable {
public:
    XojFont();
    ~XojFont() override;

public:
    std::string getName() const;
    void setName(std::string name);

    double getSize() const;
    void setSize(double size);

    void operator=(const XojFont& font);

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
