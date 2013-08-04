#include "LatexGlade.h"

#include "../../control/tools/ImageHandler.h"
#include "../../model/TexImage.h"

#include "../../util/LatexAction.h"

LatexGlade::LatexGlade(GladeSearchpath * gladeSearchPath) :
	GladeGui(gladeSearchPath, "texdialog.glade", "texDialog") {
	XOJ_INIT_TYPE(LatexGlade);

	this->theLatex = NULL;

	//GtkWidget * vbox = get("texVBox");	
	//g_return_if_fail(vbox != NULL);
	this->texBox = get("texEntry");
	gtk_entry_set_max_length(GTK_ENTRY(this->texBox),50);

	gtk_widget_show(this->texBox);
}

LatexGlade::~LatexGlade() {
	XOJ_CHECK_TYPE(LatexGlade);

	XOJ_RELEASE_TYPE(LatexGlade);

}

void LatexGlade::setTex(gchar * texString)
{
	this->theLatex = texString;
}
gchar * LatexGlade::getTex()
{
	return this->theLatex;
}

void LatexGlade::save() {
	this->theLatex = g_strdup(gtk_entry_get_text(GTK_ENTRY(this->texBox)));
}

void LatexGlade::load() {
	printf("Latex::load()\n");
	
	if (theLatex == NULL)
	{
		theLatex = "x^2";
	}
	gtk_entry_set_text(GTK_ENTRY(this->texBox), this->theLatex);


}

void LatexGlade::show(GtkWindow * parent){
	XOJ_CHECK_TYPE(LatexGlade);
	this->load();
	gtk_window_set_transient_for(GTK_WINDOW(this->window),parent);
	int res = gtk_dialog_run(GTK_DIALOG(this->window));
	if (res == 1) {
		//printf("Checkbox OK-d.\n");
		this->save();
	}
	else
	{
		this->theLatex = "";
	}
	gtk_widget_hide(this->window);
}
