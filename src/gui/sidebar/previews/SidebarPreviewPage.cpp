#include "SidebarPreviewPage.h"
#include "SidebarPreviews.h"
#include "../../Shadow.h"
#include "../../../view/PdfView.h"
#include "../../../control/Control.h"

// TODO: allow drag & drop reordering of pages

SidebarPreviewPage::SidebarPreviewPage(SidebarPreviews * sidebar, PageRef page) {
	XOJ_INIT_TYPE(SidebarPreviewPage);

	this->widget = gtk_drawing_area_new();
	gtk_widget_show(this->widget);
	g_object_ref(this->widget);

	this->crBuffer = NULL;
	this->sidebar = sidebar;
	this->page = page;
	this->selected = false;
	this->firstPainted = false;

	this->drawingMutex = g_mutex_new();

	updateSize();
	gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	g_signal_connect(this->widget, "expose_event", G_CALLBACK(exposeEventCallback), this);
	g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
}

SidebarPreviewPage::~SidebarPreviewPage() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	this->sidebar->getControl()->getScheduler()->removeSidebar(this);
	this->page = NULL;

	gtk_widget_destroy(this->widget);

	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}

	g_mutex_free(this->drawingMutex);
	this->drawingMutex = NULL;

	XOJ_RELEASE_TYPE(SidebarPreviewPage);
}

gboolean SidebarPreviewPage::exposeEventCallback(GtkWidget * widget, GdkEventExpose * event, SidebarPreviewPage * preview) {
	XOJ_CHECK_TYPE_OBJ(preview, SidebarPreviewPage);

	preview->paint();
	return true;
}

gboolean SidebarPreviewPage::mouseButtonPressCallback(GtkWidget * widget, GdkEventButton * event, SidebarPreviewPage * preview) {
	XOJ_CHECK_TYPE_OBJ(preview, SidebarPreviewPage);

	preview->sidebar->getControl()->getScrollHandler()->scrollToPage(preview->page);
	preview->sidebar->getControl()->firePageSelected(preview->page);
	return true;
}

void SidebarPreviewPage::setSelected(bool selected) {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	if (this->selected == selected) {
		return;
	}
	this->selected = selected;

	gtk_widget_queue_draw(this->widget);
}

void SidebarPreviewPage::repaint() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	sidebar->getControl()->getScheduler()->addRepaintSidebar(this);
}

void SidebarPreviewPage::paint() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	sidebar->setBackgroundWhite();

	if (!this->firstPainted) {
		if (!GDK_IS_WINDOW(widget->window)) {
			return;
		}

		this->firstPainted = true;
		gdk_window_set_background(widget->window, &widget->style->white);
		gtk_widget_queue_draw(this->widget);
		return;
	}

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	g_mutex_lock(this->drawingMutex);

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

		double zoom = sidebar->getZoom();

		cairo_t * cr2 = cairo_create(this->crBuffer);
		cairo_matrix_t defaultMatrix = { 0 };
		cairo_get_matrix(cr2, &defaultMatrix);

		cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

		cairo_scale(cr2, zoom, zoom);

		const char * txtLoading = _("Loading...");

		cairo_text_extents_t ex;
		cairo_set_source_rgb(cr2, 0.5, 0.5, 0.5);
		cairo_select_font_face(cr2, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr2, 70.0);
		cairo_text_extents(cr2, txtLoading, &ex);
		cairo_move_to(cr2, (page.getWidth() - ex.width) / 2 - ex.x_bearing, (page.getHeight() - ex.height) / 2 - ex.y_bearing);
		cairo_show_text(cr2, txtLoading);

		cairo_destroy(cr2);

		repaint();
	}

	cairo_t * cr = gdk_cairo_create(widget->window);
	cairo_set_source_surface(cr, this->crBuffer, 0, 0);
	cairo_paint(cr);

	double height = page.getHeight() * sidebar->getZoom();
	double width = page.getWidth() * sidebar->getZoom();

	if (this->selected) {
		// Draw border
		Util::cairo_set_source_rgbi(cr, sidebar->getControl()->getSettings()->getSelectionColor());
		cairo_set_line_width(cr, 2);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

		cairo_rectangle(cr, Shadow::getShadowTopLeftSize() + 0.5, Shadow::getShadowTopLeftSize() + 0.5, width + 3, height + 3);

		cairo_stroke(cr);

		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(), width + 4, height + 4);
	} else {
		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2, width, height);
	}

	cairo_destroy(cr);

	g_mutex_unlock(this->drawingMutex);
}

void SidebarPreviewPage::updateSize() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	gtk_widget_set_size_request(this->widget, getWidth(), getHeight());
}

int SidebarPreviewPage::getWidth() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	return page.getWidth() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
}

int SidebarPreviewPage::getHeight() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	return page.getHeight() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
}

GtkWidget * SidebarPreviewPage::getWidget() {
	XOJ_CHECK_TYPE(SidebarPreviewPage);

	return this->widget;
}

