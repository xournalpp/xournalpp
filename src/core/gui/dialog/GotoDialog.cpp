#include "GotoDialog.h"

#include <memory>  // for allocator

#include "gui/Builder.h"


class GladeSearchpath;

constexpr auto UI_FILE = "goto.ui";
constexpr auto UI_DIALOG_NAME = "gotoDialog";

using namespace xoj::popup;

GotoDialog::GotoDialog(GladeSearchpath* gladeSearchPath, size_t initialPage, size_t maxPage,
                       std::function<void(size_t)> callback):
        callback(callback) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    this->spinButton = GTK_SPIN_BUTTON(builder.get("spinPage"));
    gtk_spin_button_set_range(this->spinButton, 1, static_cast<double>(maxPage));
    gtk_spin_button_set_value(this->spinButton, static_cast<double>(initialPage + 1));
    gtk_editable_select_region(GTK_EDITABLE(this->spinButton), 0, -1);  // Select the entire content

    auto onSuccessCallback = G_CALLBACK(+[](GotoDialog* self) {
        self->callback(static_cast<size_t>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(self->spinButton))));
        gtk_window_close(self->window.get());
    });

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", onSuccessCallback, this);
}

GotoDialog::~GotoDialog() = default;
