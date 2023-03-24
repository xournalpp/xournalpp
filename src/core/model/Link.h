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

#include <string>

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

    void setInEditing(bool inEditing);
    bool isInEditing() const;

    void setFont(const XojFont& font);
    XojFont& getFont();

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

protected:
    void calcSize() const override;

private:
    XojFont font;
    std::string text;
    std::string url;
    bool inEditing = false;
};
