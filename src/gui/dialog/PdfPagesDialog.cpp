#include "PdfPagesDialog.h"
#include "../Shadow.h"
#include <math.h>
#include <Util.h>

#include <config.h>
#include <glib/gi18n-lib.h>

class PdfPage
{
public:

	PdfPage(XojPopplerPage* page, int index, PdfPagesDialog* dlg)
	{
		XOJ_INIT_TYPE(PdfPage);

		this->widget = gtk_drawing_area_new();
		gtk_widget_show(this->widget);
		this->crBuffer = NULL;
		this->page = page;
		this->selected = false;
		this->dlg = dlg;
		this->pageNr = index;

		updateSize();
		gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

		g_signal_connect(this->widget, "expose_event", G_CALLBACK(exposeEventCallback),
						this);
		g_signal_connect(this->widget, "button-press-event",
						G_CALLBACK(mouseButtonPressCallback), this);
	}

	virtual ~PdfPage()
	{
		XOJ_CHECK_TYPE(PdfPage);

		gtk_widget_destroy(this->widget);

		if (crBuffer)
		{
			cairo_surface_destroy(crBuffer);
			crBuffer = NULL;
		}

		XOJ_RELEASE_TYPE(PdfPage);
	}

	GtkWidget* getWidget()
	{
		XOJ_CHECK_TYPE(PdfPage);

		return this->widget;
	}

	int getWidth()
	{
		XOJ_CHECK_TYPE(PdfPage);

		return page->getWidth() * dlg->getZoom() + Shadow::getShadowBottomRightSize() +
				Shadow::getShadowTopLeftSize()
				+ 4;
	}

	int getHeight()
	{
		XOJ_CHECK_TYPE(PdfPage);

		return page->getHeight() * dlg->getZoom() + Shadow::getShadowBottomRightSize() +
				Shadow::getShadowTopLeftSize()
				+ 4;
	}

	void setSelected(bool selected)
	{
		XOJ_CHECK_TYPE(PdfPage);

		if (this->selected == selected)
		{
			return;
		}
		this->selected = selected;

		repaint();
	}

	void repaint()
	{
		XOJ_CHECK_TYPE(PdfPage);

		if (this->crBuffer)
		{
			cairo_surface_destroy(this->crBuffer);
			this->crBuffer = NULL;
		}
		gtk_widget_queue_draw(this->widget);
	}

	void updateSize()
	{
		XOJ_CHECK_TYPE(PdfPage);

		gtk_widget_set_size_request(this->widget, getWidth(), getHeight());
	}

private:
	static gboolean exposeEventCallback(GtkWidget* widget, GdkEventExpose* event,
										PdfPage* page)
	{
		XOJ_CHECK_TYPE_OBJ(page, PdfPage);

		page->paint();
		return true;
	}

	static gboolean mouseButtonPressCallback(GtkWidget* widget,
											GdkEventButton* event, PdfPage* page)
	{
		XOJ_CHECK_TYPE_OBJ(page, PdfPage);

		page->dlg->setSelected(page->pageNr);
		return true;
	}

	void paint()
	{
		XOJ_CHECK_TYPE(PdfPage);

		dlg->setBackgroundWhite();

		GtkAllocation alloc;
		gtk_widget_get_allocation(widget, &alloc);

		if (crBuffer == NULL)
		{
			crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width,
												alloc.height);

			double zoom = dlg->getZoom();

			cairo_t* cr2 = cairo_create(crBuffer);
			cairo_matrix_t defaultMatrix = { 0 };
			cairo_get_matrix(cr2, &defaultMatrix);

			cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2,
							Shadow::getShadowTopLeftSize() + 2);

			cairo_scale(cr2, zoom, zoom);

			page->render(cr2);
			cairo_set_operator(cr2, CAIRO_OPERATOR_DEST_OVER);
			cairo_set_source_rgb(cr2, 1., 1., 1.);
			cairo_paint(cr2);

			cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

			cairo_set_matrix(cr2, &defaultMatrix);

			double width = page->getWidth();
			double height = page->getHeight();
			width *= zoom;
			height *= zoom;

			cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

			cairo_set_source_rgb(cr2, 1, 1, 1);
			//left
			cairo_rectangle(cr2, 0, 0, Shadow::getShadowTopLeftSize() + 2, alloc.height);
			//top
			cairo_rectangle(cr2, 0, 0, alloc.width, Shadow::getShadowTopLeftSize() + 2);

			//right
			cairo_rectangle(cr2, alloc.width - Shadow::getShadowBottomRightSize() - 2, 0,
							Shadow::getShadowBottomRightSize() + 2, alloc.height);
			//bottom
			cairo_rectangle(cr2, 0, alloc.height - Shadow::getShadowBottomRightSize() - 2,
							alloc.width,
							Shadow::getShadowBottomRightSize() + 2);
			cairo_fill(cr2);

			cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);

			if (this->selected)
			{
				// Draw border
				Util::cairo_set_source_rgbi(cr2, dlg->getSettings()->getSelectionColor());
				cairo_set_line_width(cr2, 2.0);
				cairo_set_line_cap(cr2, CAIRO_LINE_CAP_BUTT);
				cairo_set_line_join(cr2, CAIRO_LINE_JOIN_BEVEL);

				cairo_rectangle(cr2, Shadow::getShadowTopLeftSize(),
								Shadow::getShadowTopLeftSize(), width + 3.5, height
								+ 3.5);

				cairo_stroke(cr2);

				Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize(),
								Shadow::getShadowTopLeftSize(), width + 5, height
								+ 5);
			}
			else
			{
				Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize() + 2,
								Shadow::getShadowTopLeftSize() + 2, width,
								height);
			}

			int x = 35;
			int y = 35;

			cairo_set_line_width(cr2, 2.0);
			cairo_arc(cr2, x, y, 25, 0, 2 * M_PI);
			cairo_set_source_rgba(cr2, 1, 1, 1, 0.8);
			cairo_fill_preserve(cr2);

			cairo_set_source_rgb(cr2, 0xff / 255.0, 0x66 / 255.0, 0x00 / 255.0);
			cairo_stroke(cr2);
			cairo_select_font_face(cr2, "@cairo:", CAIRO_FONT_SLANT_NORMAL,
								CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size(cr2, 16);

			char* txt = g_strdup_printf("%i", this->pageNr + 1); // Do not start page number with 0

			cairo_text_extents_t extents;
			cairo_text_extents(cr2, txt, &extents);
			cairo_move_to(cr2, x - extents.width / 2, y + extents.height / 2);
			cairo_show_text(cr2, txt);

			g_free(txt);

			cairo_destroy(cr2);
		}

		cairo_t* cr = gdk_cairo_create(widget->window);

		double width = cairo_image_surface_get_width(crBuffer);
		if (width != alloc.width)
		{
			double scale = ((double) alloc.width) / ((double) width);

			// Scale current image to fit the zoom level
			cairo_matrix_t defaultMatrix = { 0 };
			cairo_get_matrix(cr, &defaultMatrix);

			cairo_scale(cr, scale, scale);
			cairo_set_source_surface(cr, crBuffer, 0, 0);

			cairo_set_matrix(cr, &defaultMatrix);

			//		repaintLater();
		}
		else
		{
			cairo_set_source_surface(cr, crBuffer, 0, 0);
		}

		cairo_paint(cr);

		cairo_destroy(cr);
	}
private:
	XOJ_TYPE_ATTRIB;

	bool selected;
	int pageNr;

	XojPopplerPage* page;

	PdfPagesDialog* dlg;

	GtkWidget* widget;

	cairo_surface_t* crBuffer;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

PdfPagesDialog::PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc,
							   Settings* settings) :
GladeGui(gladeSearchPath, "pdfpages.glade", "pdfPagesDialog")
{

	XOJ_INIT_TYPE(PdfPagesDialog);

	this->pages = NULL;
	this->settings = settings;
	this->widget = gtk_layout_new(NULL, NULL);
	this->scrollPreview = gtk_scrolled_window_new(NULL, NULL);
	this->backgroundInitialized = false;
	this->selected = -1;
	this->lastWidth = -1;
	this->selectedPage = -1;

	this->count = doc->getPdfPageCount();
	this->usedPages = new bool[count];
	for (int i = 0; i < count; i++)
	{
		this->usedPages[i] = false;
	}

	// TODO LOW PRIO: may find a better solution... depending on screen size or so
	gtk_widget_set_size_request(this->window, 800, 600);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollPreview),
								GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollPreview),
										GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scrollPreview), widget);
	gtk_box_pack_start(GTK_BOX(get("vbox")), scrollPreview, true, true, 0);

	g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);

	for (int i = 0; i < doc->getPdfPageCount(); i++)
	{
		XojPopplerPage* p = doc->getPdfPage(i);
		PdfPage* page = new PdfPage(p, i, this);
		gtk_layout_put(GTK_LAYOUT(this->widget), page->getWidget(), 0, 0);

		this->pages = g_list_append(this->pages, page);
	}
	if (doc->getPdfPageCount() > 0)
	{
		setSelected(0);
	}

	gtk_widget_show_all(this->scrollPreview);

	layout();
	updateOkButton();

	g_signal_connect(get("cbOnlyNotUsed"), "toggled",
					G_CALLBACK(onlyNotUsedCallback), this);
	g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK(okButtonCallback),
					this);
}

PdfPagesDialog::~PdfPagesDialog()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	for (GList* l = this->pages; l != NULL; l = l->next)
	{
		delete (PdfPage*) l->data;
	}
	g_list_free(this->pages);
	this->pages = NULL;

	delete[] this->usedPages;
	this->usedPages = NULL;

	XOJ_RELEASE_TYPE(PdfPagesDialog);
}

Settings* PdfPagesDialog::getSettings()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	return this->settings;
}

void PdfPagesDialog::updateOkButton()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	PdfPage* p = (PdfPage*) g_list_nth_data(this->pages, this->selected);
	gtk_widget_set_sensitive(get("buttonOk"), p &&
							gtk_widget_get_visible(p->getWidget()));
}

void PdfPagesDialog::okButtonCallback(GtkButton* button, PdfPagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, PdfPagesDialog);

	dlg->selectedPage = dlg->selected;
}

void PdfPagesDialog::onlyNotUsedCallback(GtkToggleButton* tb,
										 PdfPagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, PdfPagesDialog);

	if (gtk_toggle_button_get_active(tb))
	{
		int i = 0;
		for (GList* l = dlg->pages; l != NULL; l = l->next)
		{
			PdfPage* p = (PdfPage*) l->data;
			gtk_widget_set_visible(p->getWidget(), !dlg->usedPages[i]);
			i++;
		}

	}
	else
	{
		gtk_widget_show_all(dlg->scrollPreview);
	}

	dlg->layout();
	dlg->updateOkButton();
}

void PdfPagesDialog::setPageUsed(int page)
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	this->usedPages[page] = true;
}

int PdfPagesDialog::getSelectedPage()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	return this->selectedPage;
}

double PdfPagesDialog::getZoom()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	return 0.25;
}

void PdfPagesDialog::sizeAllocate(GtkWidget* widget,
								  GtkRequisition* requisition, PdfPagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, PdfPagesDialog);

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(dlg->scrollPreview, &alloc);
	if (dlg->lastWidth == alloc.width)
	{
		return;
	}
	dlg->lastWidth = alloc.width;
	dlg->layout();
}

void PdfPagesDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	GtkWidget* w = get("cbOnlyNotUsed");

	int unused = 0;
	for (int i = 0; i < this->count; i++)
	{
		if (!this->usedPages[i])
		{
			unused++;
		}
	}

	char* txt;
	if (unused == 1)
	{
		txt = g_strdup(_("Show only not used pages (one unused page)"));
	}
	else
	{
		txt = g_strdup_printf(_("Show only not used pages (%i unused pages)"), unused);
	}

	gtk_button_set_label(GTK_BUTTON(w), txt);
	g_free(txt);

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

void PdfPagesDialog::setBackgroundWhite()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	if (this->backgroundInitialized)
	{
		return;
	}
	this->backgroundInitialized = true;
	gdk_window_set_background(GTK_LAYOUT(this->widget)->bin_window,
							&this->widget->style->white);
}

void PdfPagesDialog::setSelected(int selected)
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	if (this->selected == selected)
	{
		return;
	}

	int lastSelected = this->selected;
	PdfPage* p = (PdfPage*) g_list_nth_data(this->pages, selected);
	if (p)
	{
		p->setSelected(true);
		this->selected = selected;
	}
	p = (PdfPage*) g_list_nth_data(this->pages, lastSelected);
	if (p)
	{
		p->setSelected(false);
	}

	updateOkButton();
}

void PdfPagesDialog::layout()
{
	XOJ_CHECK_TYPE(PdfPagesDialog);

	double x = 0;
	double y = 0;
	double height = 0;
	double width = 0;

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(this->scrollPreview, &alloc);

	for (GList* l = this->pages; l != NULL; l = l->next)
	{
		PdfPage* p = (PdfPage*) l->data;

		if (!gtk_widget_get_visible(p->getWidget()))
		{
			continue;
		}

		if (x + p->getWidth() > alloc.width)
		{
			width = MAX(width, x);
			y += height;
			x = 0;
			height = 0;
		}

		gtk_layout_move(GTK_LAYOUT(this->widget), p->getWidget(), x, y);

		height = MAX(height, p->getHeight());

		x += p->getWidth();
	}

	gtk_layout_set_size(GTK_LAYOUT(this->widget), width, y);

}
