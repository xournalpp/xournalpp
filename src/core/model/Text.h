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

#include <string>  // for string
#include <vector>

#include <pango/pango.h>

#include "model/Element.h"
#include "util/raii/GObjectSPtr.h"

#include "AudioElement.h"  // for AudioElement
#include "Font.h"          // for XojFont

class Element;
class ObjectInputStream;
class ObjectOutputStream;
class XojPdfRectangle;

class Text: public AudioElement {
public:
    Text();
    ~Text() override;

public:
    void setFont(const XojFont& font);
    auto getFont() -> XojFont&;
    auto getFontSize() const -> double;       // same result as getFont()->getSize(), but const
    auto getFontName() const -> std::string;  // same result as getFont()->getName(), but const

    auto getText() const -> const std::string&;
    void setText(std::string text);

    void setWidth(double width);
    void setHeight(double height);

    void setInEditing(bool inEditing);
    auto isInEditing() const -> bool;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;
    void updatePangoFont(PangoLayout* layout) const;

    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    auto rescaleOnlyAspectRatio() -> bool override;

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
    auto findText(const std::string& search) const -> std::vector<XojPdfRectangle>;

private:
    XojFont font;

    std::string text;

    bool inEditing = false;
};
