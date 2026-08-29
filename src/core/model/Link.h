/*
 * Xournal++
 *
 * A link element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include <pango/pango-layout.h>  // for Layout
#include <pango/pango.h>         // for Pango

#include "util/raii/GObjectSPtr.h"  // For GObjectSPtr

#include "Font.h"  // for XojFont
#include "RectangularElement.h"
#include "TextAlignment.h"

class ObjectInputStream;
class ObjectOutputStream;

class Link: public RectangularElement {
public:
    Link();
    ~Link() override = default;

public:
    void setText(std::string text);
    std::string getText() const;
    void setUrl(std::string url);
    std::string getUrl() const;

    void setFont(const XojFont& font);
    XojFont& getFont();
    const XojFont& getFont() const;

    void setAlignment(TextAlignment alignment);
    TextAlignment getAlignment() const;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;

public:
    ElementPtr clone() const override;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    static constexpr double PADDING = 2.0;  // space around text including painted border

protected:
    void calcSize() const override;

private:
    XojFont font;
    std::string text;
    std::string url;
    TextAlignment alignment = TextAlignment::LEFT;
};
