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

#include <gtk/gtk.h>  // for GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

class GladeSearchpath;

class FillOpacityDialog: public GladeGui {
public:
    FillOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha, bool pen);
    ~FillOpacityDialog() override;

public:
    void show(GtkWindow* parent) override;

    int getResultAlpha() const;

private:
    void setPreviewImage(int alpha);

private:
    int resultAlpha = -1;
    bool pen;
};
