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

#include "AudioElement.h"  // for AudioElement
#include "Font.h"          // for XojFont

class Link: public AudioElement {
public:
    Link();

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
     * @override
     */
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    XojFont font;
    std::string text;
    std::string url;
    bool inEditing;
};