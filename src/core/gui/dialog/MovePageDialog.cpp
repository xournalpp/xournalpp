#include "MovePageDialog.h"

#include <memory>  // for allocator
#include <glib.h>  // for TRUE

class GladeSearchpath;

MovePageDialog::MovePageDialog(GladeSearchpath* gladeSearchPath, int currentPage, int maxPage):
        GladeGui(gladeSearchPath, "movePage.glade", "movePageDialog") {
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(get("movePageFrom")), 1, maxPage);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(get("movePageTo")), 1, maxPage);
    // TODO set page from to current page
}

MovePageDialog::~MovePageDialog() = default;

auto MovePageDialog::getSelectedPageFrom() const -> int { return this->selectedPageFrom; }
auto MovePageDialog::getSelectedPageTo() const -> int { return this->selectedPageTo; }

void MovePageDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_entry_set_activates_default(GTK_ENTRY(get("movePageFrom")), TRUE);
    gtk_entry_set_activates_default(GTK_ENTRY(get("movePageTo")), TRUE);

    int returnCode = gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);

    if (returnCode == GTK_RESPONSE_OK) {
        this->selectedPageFrom = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("movePageFrom")));
        this->selectedPageTo = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("movePageTo")));
    }
}
