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

#include "Element.h"  // for Element
#include "Font.h"     // for XojFont

class LinkAlignment {
public:
    enum Value { LEFT, CENTER, RIGHT };
    static constexpr std::array<const char8_t*, 3> NAMES = {u8"left", u8"center", u8"right"};
    LinkAlignment(Value v): value(v) {}

    // Implicit conversion to underlying enum type
    operator const Value&() const { return value; }
    operator Value&() { return value; }

private:
    Value value;
};

class Element;
class ObjectInputStream;
class ObjectOutputStream;

class Link: public Element {
public:
    Link();
    ~Link() override = default;

public:
    void setText(std::string text);
    std::string getText() const;

    void setTextPos(double x, double y);

    void setUrl(std::string url);
    std::string getUrl() const;

    /* A link gets highlighted by hovering with the mouse over it */
    void setHighlighted(bool highlighted);
    bool isHighlighted() const;

    /* A link gets selected / deselected by a single click on it */
    void setSelected(bool selected);
    bool isSelected() const;

    void setFont(const XojFont& font);
    XojFont& getFont();
    const XojFont& getFont() const;

    void setAlignment(LinkAlignment alignment);
    LinkAlignment getAlignment() const;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;

public:
    /**
     * @override (required)
     */
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;
    ElementPtr clone() const override;

    /**
     * @override (optional)
     */
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;
    bool rescaleOnlyAspectRatio() const override;
    bool rescaleWithMirror() const override;

    static constexpr double PADDING = 2.0;  // space around text including painted border

    //  Must match the enum LinkAlignment
    static constexpr PangoAlignment PANGO_ALIGNMENT[] = {PANGO_ALIGN_LEFT, PANGO_ALIGN_CENTER, PANGO_ALIGN_RIGHT};


protected:
    void calcSize() const override;

private:
    XojFont font;
    std::string text;
    std::string url;
    LinkAlignment alignment = LinkAlignment::LEFT;
    bool highlighted = false;
    bool selected = false;
};
