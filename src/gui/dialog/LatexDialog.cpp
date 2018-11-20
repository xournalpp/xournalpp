#include "LatexDialog.h"

LatexDialog::LatexDialog(GladeSearchpath* gladeSearchPath)
 : GladeGui(gladeSearchPath, "texdialog.glade", "texDialog")
{
	XOJ_INIT_TYPE(LatexDialog);

	this->texBox = get("texEntry");

	// increase the maximum length to something reasonable.
	gtk_entry_set_max_length(GTK_ENTRY(this->texBox), 500);

	gtk_widget_show(this->texBox);
}

LatexDialog::~LatexDialog()
{
	XOJ_RELEASE_TYPE(LatexDialog);
}

void LatexDialog::setTex(string texString)
{
	XOJ_CHECK_TYPE(LatexDialog);
	this->theLatex = texString;
}

string LatexDialog::getTex()
{
	XOJ_CHECK_TYPE(LatexDialog);
	return this->theLatex;
}

void LatexDialog::save()
{
	XOJ_CHECK_TYPE(LatexDialog);
	this->theLatex = gtk_entry_get_text(GTK_ENTRY(this->texBox));
}

void LatexDialog::load()
{
	XOJ_CHECK_TYPE(LatexDialog);

	if (theLatex.empty())
	{
		theLatex = "x^2";
	}

	gtk_entry_set_text(GTK_ENTRY(this->texBox), this->theLatex.c_str());
}

void LatexDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(LatexDialog);

	this->load();
	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	int res = gtk_dialog_run(GTK_DIALOG(this->window));
	if (res == 1)
	{
		this->save();
	}
	else
	{
		this->theLatex = "";
	}

	gtk_widget_hide(this->window);
}
