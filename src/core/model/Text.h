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
#include "util/Point.h"
#include "util/Rectangle.h"
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

    auto getPangoLayout() const -> xoj::util::GObjectSPtr<PangoLayout>;

    void scale(xoj::util::Point<double> base, double fx, double fy, bool restoreLineWidth) override;

    auto rescaleOnlyAspectRatio() -> bool override;

    auto cloneText() const -> std::unique_ptr<Text>;
    auto clone() const -> ElementPtr override;

    auto intersects(xoj::util::Point<double> pos, double halfEraserSize) const -> bool override;
    auto intersects(xoj::util::Point<double> pos, double halfEraserSize, double* gap) const -> bool override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    auto internalUpdateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> override;
    void updateSnapping() const;

public:
    auto findText(const std::string& search) const -> std::vector<XojPdfRectangle>;

private:
    // cache:
    xoj::util::GObjectSPtr<PangoLayout> layout;
    // data:
    std::string text;
    std::pair<double, double> sizes{0, 0};

    XojFont font{"Sans", 12};

    bool inEditing = false;
};
