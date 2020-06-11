#include "AboutDialog.h"

#include <config.h>
#include <gtk/gtk.h>

#include "StringUtils.h"

AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath): GladeGui(gladeSearchPath, "about.glade", "aboutDialog") {
    gtk_label_set_markup(GTK_LABEL(get("lbBuildDate")), __DATE__ ", " __TIME__);
    gtk_label_set_markup(GTK_LABEL(get("lbVersion")), PROJECT_VERSION);
    gtk_label_set_markup(GTK_LABEL(get("lbRevId")), GIT_COMMIT_ID);

    char gtkVersion[10];
    sprintf(gtkVersion, "%d.%d.%d", gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());

    gtk_label_set_markup(GTK_LABEL(get("lbGtkVersion")), gtkVersion);

    GtkWidget* w = get("vbox1");
    GtkWidget* linkButton = gtk_link_button_new("http://github.com/xournalpp/xournalpp");
    gtk_widget_show(linkButton);
    gtk_box_pack_start(GTK_BOX(w), linkButton, true, true, 0);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);
}
