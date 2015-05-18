#include "AboutDialog.h"

#include <config.h>
#include <StringUtils.h>

AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath) : GladeGui(gladeSearchPath, "about.glade", "aboutDialog")
{
	XOJ_INIT_TYPE(AboutDialog);

	GtkLabel* labelTitle = GTK_LABEL(get("labelTitle"));
	gtk_label_set_markup(labelTitle,
						 "<span size=\"xx-large\" weight=\"bold\">Xournal++ " PROJECT_VERSION "</span>\n"
						 "<i>The next generation</i>\n"
						 "Built on: " __DATE__ " at " __TIME__);

	GtkWidget* w = get("vbox1");
	GtkWidget* linkButton = gtk_link_button_new("http://github.com/xournalpp/xournalpp");
	gtk_widget_show(linkButton);
	gtk_box_pack_start_defaults(GTK_BOX(w), linkButton);

	// Authors of the application
	string authors("Denis Auroux, 2006-2010\n"
				   "Andreas Butti, 2010-2015\n"
				   "Wilson Brenna (tex support), 2012-2015\n"
				   "Marek Pikuła, 2015\n");

	w = get("lbAuthors");
	gtk_label_set_text(GTK_LABEL(w), authors.c_str());
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
