#include "XojMsgBox.h"

#include <i18n.h>


void XojMsgBox::showErrorToUser(GtkWindow* win, string msg)
{
	GtkWidget* dialog = gtk_message_dialog_new(win, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
											   "%s", msg.c_str());
	if (win != NULL)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

int XojMsgBox::replaceFileQuestion(GtkWindow* win, string msg)
{
	GtkWidget* dialog = gtk_message_dialog_new(win, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
											   "%s", msg.c_str());
	if (win != NULL)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	}
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another name"), 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Replace"), 2);
	int res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return res;
}


