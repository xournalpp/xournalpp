#include "LatexDialog.h"

LatexDialog::LatexDialog(GladeSearchpath *gladeSearchPath)
 : GladeGui(gladeSearchPath, "texdialog.glade", "texDialog")
{
	XOJ_INIT_TYPE(LatexDialog);

	this->texBox = get("texEntry");
	this->texTempRender = get("texImage");
	this->scaledRender = NULL;

	// increase the maximum length to something reasonable.
	gtk_entry_set_max_length(GTK_ENTRY(this->texBox), 500);

	// Background color for the temporary render, default is white because
	// on dark themed DE the LaTex is hard to read
	GtkCssProvider* cssProvider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(cssProvider, "*{background-color:white;padding:10px;}", -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(this->texTempRender), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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

void LatexDialog::setTempRender(cairo_surface_t* cairoTexTempRender)
{
	XOJ_CHECK_TYPE(LatexDialog);

	//If a previous render exists, destroy it
	if(this->scaledRender != NULL)
	{
		cairo_surface_destroy(this->scaledRender);
	}
	size_t length = string(gtk_entry_get_text(GTK_ENTRY(this->texBox))).size();
	//Max size = 100%, Min size = 45%
	double factor = MAX(0.45, 1 -  length / 100.0 );
	// Every time the controller updates the temporary render, we update
	// our corresponding GtkWidget	
	this->scaledRender = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cairo_image_surface_get_width(cairoTexTempRender)*factor, cairo_image_surface_get_height(cairoTexTempRender)*factor);
	cairo_t* cr = cairo_create(this->scaledRender);
	cairo_scale(cr, factor, factor);
		
	cairo_set_source_surface(cr, cairoTexTempRender, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);	

	gtk_image_set_from_surface(GTK_IMAGE(this->texTempRender), this->scaledRender);
}

GtkWidget* LatexDialog::getTexBox()
{
	XOJ_CHECK_TYPE(LatexDialog);
	return this->texBox;
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

void LatexDialog::show(GtkWindow *parent)
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
