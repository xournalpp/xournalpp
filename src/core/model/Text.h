/*
 * Xournal++
 *
 * A text element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>    // for list
#include <string>  // for string
#include <vector>  // for vector

#include <pango/pango.h>

#include "model/Element.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"  // for PangoAttrListSPtr

#include "AudioElement.h"  // for AudioElement
#include "Font.h"          // for XojFont

class Element;
class ObjectInputStream;
class ObjectOutputStream;
class XojPdfRectangle;

enum class TextAlignment { LEFT = 0, CENTER = 1, RIGHT = 2 };

class Text: public AudioElement {
public:
    Text();
    ~Text() override;

public:
    void setFont(const XojFont& font);
    XojFont& getFont();
    double getFontSize() const;       // same result as getFont()->getSize(), but const
    std::string getFontName() const;  // same result as getFont()->getName(), but const

    const std::string& getText() const;
    void setText(std::string text);

    void setWidth(double width);
    void setHeight(double height);

    void setInEditing(bool inEditing);
    bool isInEditing() const;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;
    void updatePangoFont(PangoLayout* layout) const;

    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool rescaleOnlyAspectRatio() override;

    void setAlignment(TextAlignment align);
    TextAlignment getAlignment() const;

    xoj::util::PangoAttrListSPtr getAttributeList() const;
    void addAttribute(PangoAttribute* attrib);
    void clearAttributes();
    void updateTextAttributesPosition(int pos, int del, int add);

    /**
     * Replaces the current attribute list with a copy of the given one
     * Important: Does not take ownership of the new attribute list,
     * Object has to be freed by the calling function
     */
    void replaceAttributes(xoj::util::PangoAttrListSPtr attributes);

    auto cloneText() const -> std::unique_ptr<Text>;
    auto clone() const -> ElementPtr override;

    bool intersects(double x, double y, double halfEraserSize) const override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) const override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;
    void updateSnapping() const;

public:
    std::vector<XojPdfRectangle> findText(const std::string& search) const;

private:
    XojFont font;

    std::string text;

    TextAlignment alignment = TextAlignment::LEFT;
    xoj::util::PangoAttrListSPtr attributes = nullptr;

    bool inEditing = false;
};
