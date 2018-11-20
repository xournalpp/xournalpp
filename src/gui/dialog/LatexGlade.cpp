#include "LatexGlade.h"

#include "LatexAction.h"

#include "control/tools/ImageHandler.h"
#include "model/TexImage.h"

#include <iostream>
using std::cout;
using std::endl;

LatexGlade::LatexGlade(GladeSearchpath* gladeSearchPath)
 : GladeGui(gladeSearchPath, "texdialog.glade", "texDialog")
{
	XOJ_INIT_TYPE(LatexGlade);

	this->texBox = get("texEntry");

	// increase the maximum length to something reasonable.
	gtk_entry_set_max_length(GTK_ENTRY(this->texBox), 500);

	gtk_widget_show(this->texBox);
}

LatexGlade::~LatexGlade()
{
	XOJ_RELEASE_TYPE(LatexGlade);
}

void LatexGlade::setTex(string texString)
{
	XOJ_CHECK_TYPE(LatexGlade);
	this->theLatex = texString;
}

string LatexGlade::getTex()
{
	XOJ_CHECK_TYPE(LatexGlade);
	return this->theLatex;
}

void LatexGlade::save()
{
	XOJ_CHECK_TYPE(LatexGlade);
	this->theLatex = gtk_entry_get_text(GTK_ENTRY(this->texBox));
}

void LatexGlade::load()
{
	XOJ_CHECK_TYPE(LatexGlade);

	if (theLatex.empty())
	{
		theLatex = "x^2";
	}

	gtk_entry_set_text(GTK_ENTRY(this->texBox), this->theLatex.c_str());
}

void LatexGlade::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(LatexGlade);

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
