#include "AboutDialog.h"

#include <cstdio>  // for sprintf
#include <memory>  // for allocator

#include <gtk/gtk.h>  // for gtk_box_pack_start, gtk_label_set_markup

#include "util/i18n.h"  // for _

#include "config-git.h"  // for GIT_COMMIT_ID
#include "config.h"      // for PROJECT_VERSION

class GladeSearchpath;

AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath): GladeGui(gladeSearchPath, "about.glade", "aboutDialog") {
    gtk_label_set_markup(GTK_LABEL(get("lbBuildDate")), __DATE__ ", " __TIME__);
    gtk_label_set_markup(GTK_LABEL(get("lbVersion")), PROJECT_VERSION);
    gtk_label_set_markup(GTK_LABEL(get("lbRevId")), GIT_COMMIT_ID);

    char gtkVersion[10];
    sprintf(gtkVersion, "%u.%u.%u", gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());

    gtk_label_set_markup(GTK_LABEL(get("lbGtkVersion")), gtkVersion);

    auto w1 = get("vboxRepo");
    auto linkButton1 = gtk_link_button_new("https://github.com/xournalpp/xournalpp");
    gtk_widget_show(linkButton1);
    gtk_box_pack_start(GTK_BOX(w1), linkButton1, true, true, 0);

    auto w2 = get("vboxWebsite");
    auto linkButton2 = gtk_link_button_new("https://xournalpp.github.io");
    gtk_widget_show(linkButton2);
    gtk_box_pack_start(GTK_BOX(w2), linkButton2, true, true, 0);


    auto w3 = get("vboxCommunity");
    auto linkButton3 =
            gtk_link_button_new_with_label("https://raw.githubusercontent.com/xournalpp/xournalpp/master/AUTHORS",
                                           _("See the full list of contributors"));
    gtk_widget_show(linkButton3);
    gtk_box_pack_start(GTK_BOX(w3), linkButton3, true, true, 0);


    auto w4 = get("vboxLicense");
    auto linkButton4 = gtk_link_button_new_with_label(
            "https://raw.githubusercontent.com/xournalpp/xournalpp/master/LICENSE", _("GNU GPLv2 or later"));
    gtk_widget_show(linkButton4);
    gtk_box_pack_start(GTK_BOX(w4), linkButton4, true, true, 0);
}


AboutDialog::~AboutDialog() = default;

void AboutDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);
}
