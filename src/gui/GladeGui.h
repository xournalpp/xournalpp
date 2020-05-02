/*
 * Xournal++
 *
 * Abstract GUI class, which loads the glade objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <gtk/gtk.h>


class GladeGui {
public:
    GladeGui(const std::string& glade);
    ~GladeGui();

    GtkWidget* get(const std::string& name);

    GtkBuilder* getBuilder();

private:
    /**
     * The Glade resources
     */
    GtkBuilder* builder;
};
