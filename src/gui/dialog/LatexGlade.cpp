#include "LatexGlade.h"

#include "LatexAction.h"

#include "control/tools/ImageHandler.h"
#include "model/TexImage.h"

#include <iostream>
using std::cout;
using std::endl;

LatexGlade::LatexGlade(GladeSearchpath* gladeSearchPath) : GladeGui(gladeSearchPath, "texdialog.glade", "texDialog")
{
	XOJ_INIT_TYPE(LatexGlade);

	//GtkWidget * vbox = get("texVBox");
	//g_return_if_fail(vbox != NULL);
	this->texBox = get("texEntry");
	//gtk_entry_set_max_length(GTK_ENTRY(this->texBox),50);
	//increase the maximum length to something reasonable.
	gtk_entry_set_max_length(GTK_ENTRY(this->texBox), 500);

	gtk_widget_show(this->texBox);
}

LatexGlade::~LatexGlade()
{
	XOJ_CHECK_TYPE(LatexGlade);

	XOJ_RELEASE_TYPE(LatexGlade);

}

void LatexGlade::setTex(string texString)
{
	this->theLatex = texString;
}

string LatexGlade::getTex()
{
	return this->theLatex;
}

void LatexGlade::save()
{
	this->theLatex = gtk_entry_get_text(GTK_ENTRY(this->texBox));
}

void LatexGlade::load()
{
	cout << "Latex::load()" << endl;

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
		//cout << "Checkbox OK-d." << endl;
		this->save();
	}
	else
	{
		this->theLatex = "";
	}
	gtk_widget_hide(this->window);
}
