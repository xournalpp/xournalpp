#include "BaseElementView.h"

#include "gui/Shadow.h"

BaseElementView::BaseElementView()
 : crBuffer(NULL), selected(false)
{
	XOJ_INIT_TYPE(BaseElementView);

//	this->widget = gtk_label_new("XXXXXXXXXXXXXXXXXx");
	this->widget = gtk_drawing_area_new();
	gtk_widget_show(this->widget);

	gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
	g_signal_connect(this->widget, "draw", G_CALLBACK(drawCallback), this);
//	g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
}

BaseElementView::~BaseElementView()
{
	XOJ_CHECK_TYPE(BaseElementView);

	gtk_widget_destroy(this->widget);

	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}

	XOJ_RELEASE_TYPE(BaseElementView);
}

gboolean BaseElementView::drawCallback(GtkWidget* widget, cairo_t* cr, BaseElementView* element)
{
	XOJ_CHECK_TYPE_OBJ(element, BaseElementView);

	element->paint(cr);
	return true;
}

void BaseElementView::paint(cairo_t* cr)
{
	XOJ_CHECK_TYPE(BaseElementView);

	printf("**paint**\n");


//	this->dlg->setBackgroundWhite();

	GtkAllocation alloc;
	gtk_widget_get_allocation(this->widget, &alloc);

	if (this->crBuffer == NULL)
	{
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

		cairo_t* cr2 = cairo_create(this->crBuffer);
		cairo_matrix_t defaultMatrix = { 0 };
		cairo_get_matrix(cr2, &defaultMatrix);

		cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

		/*
		cairo_scale(cr2, this->zoom, this->zoom);

		GdkPixbuf* p = this->backgroundImage.getPixbuf();
		gdk_cairo_set_source_pixbuf(cr2, p, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
		cairo_paint(cr2);
		*/

		cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

		cairo_set_matrix(cr2, &defaultMatrix);

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
						alloc.width, Shadow::getShadowBottomRightSize() + 2);
		cairo_fill(cr2);

		cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);

		if (this->selected)
		{
			// Draw border
			// TODO !!!!!!!! Util::cairo_set_source_rgbi(cr2, dlg->getSettings()->getSelectionColor());
			cairo_set_line_width(cr2, 2);
			cairo_set_line_cap(cr2, CAIRO_LINE_CAP_BUTT);
			cairo_set_line_join(cr2, CAIRO_LINE_JOIN_BEVEL);

			cairo_rectangle(cr2, Shadow::getShadowTopLeftSize() + 1.5,
							Shadow::getShadowTopLeftSize() + 1.5, alloc.width + 2, alloc.height + 2);

			cairo_stroke(cr2);

			Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize(),
							   Shadow::getShadowTopLeftSize(), alloc.width + 4, alloc.height + 4);
		}
		else
		{
			Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize() + 2,
							   Shadow::getShadowTopLeftSize() + 2, alloc.width, alloc.height);
		}

		cairo_destroy(cr2);
	}

	cairo_set_source_surface(cr, this->crBuffer, 0, 0);
	cairo_paint(cr);
}

GtkWidget* BaseElementView::getWidget()
{
	XOJ_CHECK_TYPE(BaseElementView);

	updateSize();
	return this->widget;
}

int BaseElementView::getWidth()
{
	XOJ_CHECK_TYPE(BaseElementView);

//	calcSize();
//	return width + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;

	return 100;
}

int BaseElementView::getHeight()
{
	XOJ_CHECK_TYPE(BaseElementView);

//	calcSize();
//	return height + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
	return 100;
}

void BaseElementView::updateSize()
{
	XOJ_CHECK_TYPE(BaseElementView);

	gtk_widget_set_size_request(this->widget, this->getWidth(), this->getHeight());
}

