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

class AboutDialog: public GladeGui {
public:
    AboutDialog(GladeSearchpath* gladeSearchPath);
    ~AboutDialog() override;

public:
    void show(GtkWindow* parent) override;
};
