/*
 * Xournal++
 *
 * Displays a TexImage Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "View.h"

class TexImage;

class xoj::view::TexImageView: public xoj::view::ElementView {
public:
    TexImageView(const TexImage* texImage);
    virtual ~TexImageView();

    /**
     * Draws a TexImage model
     */
    void draw(const Context& ctx) const override;

private:
    const TexImage* texImage;
};
