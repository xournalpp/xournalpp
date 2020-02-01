#include "GotoDialog.h"

GotoDialog::GotoDialog(GladeSearchpath* gladeSearchPath, int maxPage)
 : GladeGui(gladeSearchPath, "goto.glade", "gotoDialog")
{
	XOJ_INIT_TYPE(GotoDialog);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(get("spinPage")), 1, maxPage);
}

GotoDialog::~GotoDialog()
{
	XOJ_RELEASE_TYPE(GotoDialog);
}

int GotoDialog::getSelectedPage()
{
	XOJ_CHECK_TYPE(GotoDialog);

	return this->selectedPage;
}

void GotoDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(GotoDialog);

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	gtk_entry_set_activates_default(GTK_ENTRY(get("spinPage")), TRUE);

	int returnCode = gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);

	if (returnCode == GTK_RESPONSE_OK)
	{
		this->selectedPage = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinPage")));
	}
}
