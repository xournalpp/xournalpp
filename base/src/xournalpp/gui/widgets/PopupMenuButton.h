/*
 * Xournal++
 *
 * Button with Popup Menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "XournalType.h"

class PopupMenuButton {
public:
    PopupMenuButton(GtkWidget* button, GtkWidget* menu);
    virtual ~PopupMenuButton();

public:
    void setMenu(GtkWidget* menu);

private:
    GtkWidget* button;
    GtkWidget* menu;
};
