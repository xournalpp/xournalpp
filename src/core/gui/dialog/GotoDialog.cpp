#include "GotoDialog.h"

#include <gtk/gtk.h>

#include "GtkDialogUtil.h"

GotoDialog::GotoDialog(GladeSearchpath* gladeSearchPath, int maxPage):
        GladeGui(gladeSearchPath, "goto.glade", "gotoDialog") {
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(get("spinPage")), 1, maxPage);
}

GotoDialog::~GotoDialog() = default;

auto GotoDialog::getSelectedPage() const -> int { return this->selectedPage; }

void GotoDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_entry_set_activates_default(GTK_ENTRY(get("spinPage")), TRUE);

    auto returnCode = wait_for_gtk_dialog_result(GTK_DIALOG(this->window));

    if (returnCode == GTK_RESPONSE_OK) {
        this->selectedPage = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get("spinPage")));
    }
    gtk_widget_hide(this->window);
}
