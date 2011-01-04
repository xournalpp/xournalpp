#include "SidebarPreview.h"
#include "../Shadow.h"
#include "Sidebar.h"

SidebarPreview::SidebarPreview(Sidebar * sidebar, XojPage * page) {
	this->widget = gtk_drawing_area_new();
	gtk_widget_show(this->widget);
	this->crBuffer = NULL;
	this->sidebar = sidebar;
	this->page = page;
	this->view = new DocumentView();
	this->selected = false;

	updateSize();
	gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

	g_signal_connect(this->widget, "expose_event", G_CALLBACK(exposeEventCallback), this);
	g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
}

SidebarPreview::~SidebarPreview() {
	gtk_widget_destroy(this->widget);
	delete this->view;

	if (crBuffer) {
		cairo_surface_destroy(crBuffer);
		crBuffer = NULL;
	}
}

gboolean SidebarPreview::exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, SidebarPreview * preview) {
	preview->paint();
	return true;
}

gboolean SidebarPreview::mouseButtonPressCallback(GtkWidget *widget, GdkEventButton *event, SidebarPreview * preview) {
	preview->sidebar->getControl()->scrollToPage(preview->page);
	return true;
}

void SidebarPreview::setSelected(bool selected) {
	if (this->selected == selected) {
		return;
	}
	this->selected = selected;

	repaint();
}

void SidebarPreview::repaint() {
	if (crBuffer) {
		cairo_surface_destroy(crBuffer);
		crBuffer = NULL;
	}
	gtk_widget_queue_draw(this->widget);
}

void SidebarPreview::paint() {
	sidebar->setBackgroundWhite();

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	if (crBuffer == NULL) {
		crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

		double zoom = sidebar->getZoom();

		cairo_t * cr2 = cairo_create(crBuffer);
		cairo_matrix_t defaultMatrix = { 0 };
		cairo_get_matrix(cr2, &defaultMatrix);

		cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

		cairo_scale(cr2, zoom, zoom);

		PopplerPage * popplerPage = NULL;

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = page->getPdfPageNr();
			popplerPage = sidebar->getDocument()->getPdfPage(pgNo);
		}

		view->drawPage(page, popplerPage, cr2);

		cairo_set_matrix(cr2, &defaultMatrix);

		double height = page->getHeight() * sidebar->getZoom();
		double width = page->getWidth() * sidebar->getZoom();

		cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

		cairo_set_source_rgb(cr2, 1, 1, 1);
		cairo_rectangle(cr2, 0, 0, Shadow::getShadowTopLeftSize() + 2, alloc.height);
		cairo_rectangle(cr2, 0, 0, alloc.height, Shadow::getShadowTopLeftSize() + 2);

		cairo_rectangle(cr2, alloc.width - Shadow::getShadowBottomRightSize() - 2, 0,
				Shadow::getShadowBottomRightSize() + 2, alloc.height);
		cairo_rectangle(cr2, 0, alloc.height - Shadow::getShadowBottomRightSize() - 2, alloc.width,
				Shadow::getShadowBottomRightSize() + 2);
		cairo_fill(cr2);

		cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);

		if (this->selected) {
			// Draw border
			cairo_set_source_rgb(cr2, 1, 0, 0);
			cairo_set_line_width(cr2, 2.0);
			cairo_set_line_cap(cr2, CAIRO_LINE_CAP_BUTT);
			cairo_set_line_join(cr2, CAIRO_LINE_JOIN_BEVEL);

			cairo_rectangle(cr2, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(), width + 3, height + 3);

			cairo_stroke(cr2);

			Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(), width + 4, height
					+ 4, 0, 0, 0);
		} else {
			Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2, width,
					height, 0, 0, 0);
		}

		cairo_destroy(cr2);
	}

	cairo_t * cr = gdk_cairo_create(widget->window);

	double width = cairo_image_surface_get_width(crBuffer);
	if (width != alloc.width) {
		double scale = ((double) alloc.width) / ((double) width);

		// Scale current image to fit the zoom level
		cairo_matrix_t defaultMatrix = { 0 };
		cairo_get_matrix(cr, &defaultMatrix);

		cairo_scale(cr, scale, scale);
		cairo_set_source_surface(cr, crBuffer, 0, 0);

		cairo_set_matrix(cr, &defaultMatrix);

		//		repaintLater();
	} else {
		cairo_set_source_surface(cr, crBuffer, 0, 0);
	}

	cairo_paint(cr);

	cairo_destroy(cr);
}

void SidebarPreview::updateSize() {
	gtk_widget_set_size_request(widget, getWidth(), getHeight());
}

int SidebarPreview::getWidth() {
	return page->getWidth() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize()
			+ 4;
}

int SidebarPreview::getHeight() {
	return page->getHeight() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize()
			+ 4;
}

GtkWidget * SidebarPreview::getWidget() {
	return this->widget;
}

