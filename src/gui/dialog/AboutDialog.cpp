#include "AboutDialog.h"

#include <config.h>
#include <StringUtils.h>

AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath) : GladeGui(gladeSearchPath, "about.glade", "aboutDialog")
{
	XOJ_INIT_TYPE(AboutDialog);

	gtk_label_set_markup(GTK_LABEL(get("lbBuildDate")), __DATE__ ", " __TIME__);
	gtk_label_set_markup(GTK_LABEL(get("lbVersion")), PROJECT_VERSION);

	GtkWidget* w = get("vbox1");
	GtkWidget* linkButton = gtk_link_button_new("http://github.com/xournalpp/xournalpp");
	gtk_widget_show(linkButton);
	gtk_box_pack_start(GTK_BOX(w), linkButton, TRUE, TRUE, 0);
}

AboutDialog::~AboutDialog()
{
	XOJ_RELEASE_TYPE(AboutDialog);
}

void AboutDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(AboutDialog);

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}
