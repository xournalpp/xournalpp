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

#include <gtk/gtk.h>

#include "AudioElement.h"
#include "Element.h"
#include "Font.h"

class Text: public AudioElement {
public:
    Text();
    ~Text() override;

public:
    void setFont(const XojFont& font);
    XojFont& getFont();
    double getFontSize() const;  // same result as getFont()->getSize(), but const
    string getFontName() const;  // same result as getFont()->getName(), but const

    string getText() const;
    void setText(string text);

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
    Element* clone() override;

    bool intersects(double x, double y, double halfEraserSize) override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) override;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;
    void updateSnapping() const;

private:
    XojFont font;

    string text;

    bool inEditing = false;
};
