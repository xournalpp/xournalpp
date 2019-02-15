#include "LatexDialog.h"

LatexDialog::LatexDialog(GladeSearchpath *gladeSearchPath)
 : GladeGui(gladeSearchPath, "texdialog.glade", "texDialog")
{
	XOJ_INIT_TYPE(LatexDialog);

	this->texBox = get("texView");
	this->textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->texBox));
	this->texTempRender = get("texImage");
	this->scaledRender = NULL;

	// increase the maximum length to something reasonable.
	//gtk_entry_set_max_length(GTK_TEXT_BUFFER(this->texBox), 500);

	// Background color for the temporary render, default is white because
	// on dark themed DE the LaTex is hard to read
	this->cssProvider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(this->cssProvider, "*{background-color:white;padding:10px;}", -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(this->texTempRender), GTK_STYLE_PROVIDER(this->cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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

void LatexDialog::setTempRender(PopplerDocument* pdf, size_t length)
{
	XOJ_CHECK_TYPE(LatexDialog);

	if (poppler_document_get_n_pages(pdf) < 1)
	{
		return;
	}

	// If a previous render exists, destroy it
	if (this->scaledRender != NULL)
	{
		cairo_surface_destroy(this->scaledRender);
		this->scaledRender = NULL;
	}

	PopplerPage* page = poppler_document_get_page(pdf, 0);

	double zoom = 5;
	double pageWidth = 0;
	double pageHeight = 0;
	poppler_page_get_size(page, &pageWidth, &pageHeight);

	if ((pageWidth * zoom) > 1200)
	{
		zoom = 1200 / pageWidth;
	}

	this->scaledRender = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)(pageWidth * zoom), (int)(pageHeight * zoom));
	cairo_t* cr = cairo_create(this->scaledRender);

	cairo_scale(cr, zoom, zoom);

	poppler_page_render(page, cr);

	cairo_destroy(cr);

	// Update GTK widget
	gtk_image_set_from_surface(GTK_IMAGE(this->texTempRender), this->scaledRender);
}

GtkTextBuffer* LatexDialog::getTextBuffer()
{
	XOJ_CHECK_TYPE(LatexDialog);
	return this->textBuffer;
}


void LatexDialog::save()
{
	XOJ_CHECK_TYPE(LatexDialog);

	//I don't understand why this doesn't works 
	//if start and end are declared as pointers
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
	this->theLatex = gtk_text_buffer_get_slice(this->textBuffer, &start, &end, FALSE);
}

void LatexDialog::load()
{
	XOJ_CHECK_TYPE(LatexDialog);

	if (theLatex.empty())
	{
		theLatex = "x^2";
	}
	
	gtk_text_buffer_set_text(this->textBuffer, this->theLatex.c_str(), -1);
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
