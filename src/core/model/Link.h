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

    void setAlignment(PangoAlignment alignment);
    PangoAlignment getAlignment();

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;

public:
    /**
     * @override (required)
     */
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;
    Link* clone() const override;

    /**
     * @override (optional)
     */
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;
    bool rescaleOnlyAspectRatio() override;
    bool rescaleWithMirror() override;

    static constexpr double PADDING = 2.0;

protected:
    void calcSize() const override;
    void updateSnapping() const;

private:
    XojFont font;
    std::string text;
    std::string url;
    PangoAlignment alignment;
    bool highlighted = false;
    bool selected = false;
};
