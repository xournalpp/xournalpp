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

#include "AudioElement.h"  // for AudioElement
#include "Font.h"          // for XojFont

class Element;
class ObjectInputStream;
class ObjectOutputStream;

class Text: public AudioElement {
public:
    Text();
    ~Text() override;

public:
    void setFont(const XojFont& font);
    XojFont& getFont();
    double getFontSize() const;       // same result as getFont()->getSize(), but const
    std::string getFontName() const;  // same result as getFont()->getName(), but const

    std::string getText() const;
    void setText(std::string text);

    void setWidth(double width);
    void setHeight(double height);

    void setInEditing(bool inEditing);
    bool isInEditing() const;

    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool rescaleOnlyAspectRatio() override;

    /**
     * @overwrite
     */
    Element* clone() const override;

    bool intersects(double x, double y, double halfEraserSize) const override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) const override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;
    void updateSnapping() const;

private:
    XojFont font;

    std::string text;

    bool inEditing = false;
};
