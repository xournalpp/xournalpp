#include "ExportDialog.h"

#include <config.h>

#include "PageRange.h"
#include "i18n.h"

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "exportSettings.glade", "exportDialog") {
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spDpi")), 300);

    g_signal_connect(get("rdRangePages"), "toggled", G_CALLBACK(+[](GtkToggleButton* togglebutton, ExportDialog* self) {
                         gtk_widget_set_sensitive(self->get("txtPages"), gtk_toggle_button_get_active(togglebutton));
                     }),
                     this);
}

ExportDialog::~ExportDialog() = default;

void ExportDialog::initPages(int current, int count) {
    string allPages = "1 - " + std::to_string(count);
    gtk_label_set_text(GTK_LABEL(get("lbAllPagesInfo")), allPages.c_str());
    string currentPages = std::to_string(current);
    gtk_label_set_text(GTK_LABEL(get("lbCurrentPage")), currentPages.c_str());

    this->currentPage = current;
    this->pageCount = count;
}

void ExportDialog::removeDpiSelection() {
    gtk_widget_hide(get("lbResolution"));
    gtk_widget_hide(get("spDpi"));
    gtk_widget_hide(get("lbDpi"));
}

auto ExportDialog::getPngDpi() -> int { return gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spDpi"))); }

auto ExportDialog::isConfirmed() const -> bool { return this->confirmed; }

auto ExportDialog::getRange() -> PageRangeVector {
    GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
    GtkWidget* rdRangePages = get("rdRangePages");

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages))) {
        return PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))));
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent))) {
        PageRangeVector range;
        range.push_back(new PageRangeEntry(this->currentPage, this->currentPage));
        return range;
    }


    PageRangeVector range;
    range.push_back(new PageRangeEntry(0, this->pageCount - 1));
    return range;
}

void ExportDialog::show(GtkWindow* parent) {
    confirmed = false;

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);

    int res = gtk_dialog_run(GTK_DIALOG(this->window));

    // Button 1 OK
    if (res == 1) {
        confirmed = true;
    }

    gtk_widget_hide(this->window);
}
