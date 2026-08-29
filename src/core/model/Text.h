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

#include "util/Point.h"
#include "util/raii/GObjectSPtr.h"

#include "AudioContent.h"
#include "Font.h"  // for XojFont
#include "RectangularElement.h"
#include "TextAlignment.h"

class ObjectInputStream;
class ObjectOutputStream;
class XojPdfRectangle;

class Text: public RectangularElement, public AudioContent {
public:
    Text();
    ~Text() override;

    static constexpr double NO_WRAP = -1;

public:
    void setFont(const XojFont& font);
    XojFont& getFont();
    const XojFont& getFont() const;
    double getFontSize() const;       // same result as getFont()->getSize(), but const
    std::string getFontName() const;  // same result as getFont()->getName(), but const

    const std::string& getText() const;
    void setText(std::string text);

    void setInEditing(bool inEditing);
    bool isInEditing() const;

    xoj::util::GObjectSPtr<PangoLayout> createPangoLayout() const;
    void updatePangoFont(PangoLayout* layout) const;

    /// Set the text's wrapping width. Use Text::NO_WRAP to disable wrapping
    void setWrap(double wrap);

    /// Return Text::NO_WRAP if wrapping is disabled
    inline double getWrap() const { return wrapWidth; }

    void setAlignment(TextAlignment a);
    inline TextAlignment getAlign() const { return align; }

    inline void setJustify(bool j) { this->justify = j; }
    inline bool getJustify() const { return justify; }

    auto cloneText() const -> std::unique_ptr<Text>;
    auto clone() const -> ElementPtr override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    struct Boxes {
        xoj::util::Size<double> theoreticalSize;
        /// Could be larger or smaller than the theoretical size, and there could be an offset
        xoj::util::Rectangle<double> effectiveBounds;
    };
    /// Get the boxes out of a Pango context, ignoring any affine transformation
    static Boxes computeBoxesForLayout(PangoLayout* layout, double wrapWidth);

protected:
    void calcSize() const override;

public:
    std::vector<XojPdfRectangle> findText(const std::string& search) const;

private:
    XojFont font;

    std::string text;

    /**
     * The size of the Pango layout, without any affine transformation applied (so in Text element coordinates)
     *
     * Could be larger or smaller than the theoretical size this->naturalSize, and there could be an offset
     */
    mutable xoj::util::Rectangle<double> effectiveBounds;

    double wrapWidth = NO_WRAP;  ///< NO_WRAP for no wrap
    TextAlignment align = TextAlignment::LEFT;
    bool justify = false;  ///< Stretch whitespaces to make all complete lines have the same width

    bool inEditing = false;
};
