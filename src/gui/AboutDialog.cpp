#include "AboutDialog.h"
#include <config.h>
#include "../model/String.h"

#define AUTOHOR(name) authors += name; authors += "\n";
#define CONTRIBUTOR(name) contributors += name; contributors += "\n";

AboutDialog::AboutDialog() :
	GladeGui("about.glade", "aboutDialog") {
	GtkLabel *labelTitle;
	labelTitle = GTK_LABEL(get("labelTitle"));
	gtk_label_set_markup(labelTitle, "<span size=\"xx-large\" weight=\"bold\">Xournal++ " VERSION "</span>");

	GtkWidget * w = get("vbox1");
	GtkWidget * linkButton = gtk_link_button_new("http://xournal.sourceforge.net/");
	gtk_widget_show(linkButton);
	gtk_box_pack_start_defaults(GTK_BOX(w), linkButton);

	String authors = "";
	String contributors = "";

	// Authors of the application
	AUTOHOR("Denis Auroux");
	AUTOHOR("Andreas Butti");

	// Contributors, please insert in alphabetically order
	// TODO: insert contributors
	CONTRIBUTOR("Test1");
	CONTRIBUTOR("Test2");

	w = get("lbAuthors");
	gtk_label_set_text(GTK_LABEL(w), authors.c_str());

	w = get("lbContributors");
	gtk_label_set_text(GTK_LABEL(w), contributors.c_str());
}

AboutDialog::~AboutDialog() {
}

void AboutDialog::show() {
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

