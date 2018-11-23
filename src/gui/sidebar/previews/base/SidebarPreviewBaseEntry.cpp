#include "SidebarPreviewBaseEntry.h"

#include "SidebarPreviewBase.h"

#include "control/Control.h"
#include "gui/Shadow.h"
#include "view/PdfView.h"

#include <i18n.h>

SidebarPreviewBaseEntry::SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, PageRef page)
{
	XOJ_INIT_TYPE(SidebarPreviewBaseEntry);

	this->widget = gtk_drawing_area_new();
	gtk_widget_show(this->widget);
	g_object_ref(this->widget);

	this->crBuffer = NULL;
	this->sidebar = sidebar;
	this->page = page;
	this->selected = false;
	this->firstPainted = false;

	g_mutex_init(&this->drawingMutex);

	updateSize();
	gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	g_signal_connect(this->widget, "draw", G_CALLBACK(drawCallback), this);
	g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
}

SidebarPreviewBaseEntry::~SidebarPreviewBaseEntry()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	this->sidebar->getControl()->getScheduler()->removeSidebar(this);
	this->page = NULL;

	gtk_widget_destroy(this->widget);

	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}

	XOJ_RELEASE_TYPE(SidebarPreviewBaseEntry);
}

gboolean SidebarPreviewBaseEntry::drawCallback(GtkWidget* widget, cairo_t* cr, SidebarPreviewBaseEntry* preview)
{
	XOJ_CHECK_TYPE_OBJ(preview, SidebarPreviewBaseEntry);

	preview->paint(cr);
	return TRUE;
}

gboolean SidebarPreviewBaseEntry::mouseButtonPressCallback(GtkWidget* widget, GdkEventButton* event, SidebarPreviewBaseEntry* preview)
{
	XOJ_CHECK_TYPE_OBJ(preview, SidebarPreviewBaseEntry);

	preview->sidebar->getControl()->getScrollHandler()->scrollToPage(preview->page);
	preview->sidebar->getControl()->firePageSelected(preview->page);
	return true;
}

void SidebarPreviewBaseEntry::setSelected(bool selected)
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	if (this->selected == selected)
	{
		return;
	}
	this->selected = selected;

	gtk_widget_queue_draw(this->widget);
}

void SidebarPreviewBaseEntry::repaint()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	sidebar->getControl()->getScheduler()->addRepaintSidebar(this);
}

void SidebarPreviewBaseEntry::paint(cairo_t* cr)
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	bool doRepaint = false;

	sidebar->setBackgroundWhite();

	if (!this->firstPainted)
	{
		if (!GDK_IS_WINDOW(gtk_widget_get_window(widget)))
		{
			return;
		}

		this->firstPainted = true;
		gdk_window_set_background(gtk_widget_get_window(widget),
		                          &gtk_widget_get_style(widget)->white);
		gtk_widget_queue_draw(this->widget);
		return;
	}

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	g_mutex_lock(&this->drawingMutex);

	if (this->crBuffer == NULL)
	{
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width,
		                                            alloc.height);

		double zoom = sidebar->getZoom();

		cairo_t* cr2 = cairo_create(this->crBuffer);
		cairo_matrix_t defaultMatrix = { 0 };
		cairo_get_matrix(cr2, &defaultMatrix);

		cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2,
		                Shadow::getShadowTopLeftSize() + 2);

		cairo_scale(cr2, zoom, zoom);

		std::string txtLoading = _("Loading...");

		cairo_text_extents_t ex;
		cairo_set_source_rgb(cr2, 0.5, 0.5, 0.5);
		cairo_select_font_face(cr2, "Sans", CAIRO_FONT_SLANT_NORMAL,
		                       CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr2, 70.0);
		cairo_text_extents(cr2, txtLoading.c_str(), &ex);
		cairo_move_to(cr2, (page->getWidth() - ex.width) / 2 - ex.x_bearing,
		              (page->getHeight() - ex.height) / 2 - ex.y_bearing);
		cairo_show_text(cr2, txtLoading.c_str());

		cairo_destroy(cr2);

		doRepaint = true;
	}

	cairo_set_source_surface(cr, this->crBuffer, 0, 0);
	cairo_paint(cr);

	double height = page->getHeight() * sidebar->getZoom();
	double width = page->getWidth() * sidebar->getZoom();

	if (this->selected)
	{
		// Draw border
		Util::cairo_set_source_rgbi(cr,
		                            sidebar->getControl()->getSettings()->getSelectionColor());
		cairo_set_line_width(cr, 2);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

		cairo_rectangle(cr, Shadow::getShadowTopLeftSize() + 0.5,
		                Shadow::getShadowTopLeftSize() + 0.5, width + 3, height + 3);

		cairo_stroke(cr);

		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize(),
		                   Shadow::getShadowTopLeftSize(), width + 4, height + 4);
	}
	else
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize() + 2,
		                   Shadow::getShadowTopLeftSize() + 2, width, height);
	}

	g_mutex_unlock(&this->drawingMutex);

	if (doRepaint)
	{
		repaint();
	}
}

void SidebarPreviewBaseEntry::updateSize()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	gtk_widget_set_size_request(this->widget, getWidth(), getHeight());
}

int SidebarPreviewBaseEntry::getWidth()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	return page->getWidth() * sidebar->getZoom()
			+ Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
}

int SidebarPreviewBaseEntry::getHeight()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	return page->getHeight() * sidebar->getZoom()
			+ Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
}

GtkWidget* SidebarPreviewBaseEntry::getWidget()
{
	XOJ_CHECK_TYPE(SidebarPreviewBaseEntry);

	return this->widget;
}
