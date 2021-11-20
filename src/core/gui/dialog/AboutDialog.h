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

static void show_about_dialog(GtkWindow* parent);

class AboutDialog: public GladeGui {
public:
    AboutDialog(GladeSearchpath* gladeSearchPath);
    ~AboutDialog() override;

public:
    void show(GtkWindow* parent) override;

private:
};
