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

class CustomThicknessDialog: public GladeGui {
public:
    CustomThicknessDialog(GladeSearchpath* gladeSearchPath, int thickness);
    virtual ~CustomThicknessDialog();

public:
    virtual void show(GtkWindow* parent);

    double getResultThickness() const;

private:
    void setPreviewImage(int thickness);

private:
    int resultThickness = -1;
};
