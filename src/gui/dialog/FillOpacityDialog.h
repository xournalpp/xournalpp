/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

class FillOpacityDialog: public GladeGui {
public:
    FillOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha);
    virtual ~FillOpacityDialog();

public:
    virtual void show(GtkWindow* parent);

    int getResultAlpha() const;

private:
    void setPreviewImage(int alpha);

private:
    int resultAlpha = -1;
};
