/*
 * Xournal++
 *
 * The thickness selection dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cmath>

#include "gui/GladeGui.h"

class CustomThicknessDialog: public GladeGui {
public:
    CustomThicknessDialog(GladeSearchpath* gladeSearchPath, double thickness);
    virtual ~CustomThicknessDialog();

public:
    virtual void show(GtkWindow* parent);

    double getResultThickness() const;

private:
    double resultThickness = NAN;
};
