#include "ToolZoomSlider.h"

#include "control/zoom/ZoomControl.h"

#include <config.h>
#include <i18n.h>
#include <StringUtils.h>
#include <cmath>

ToolZoomSlider::ToolZoomSlider(ActionHandler* handler, string id, ActionType type, ZoomControl* zoom)
 : AbstractToolItem(id, handler, type, NULL),
   zoom(zoom)
{
	XOJ_INIT_TYPE(ToolZoomSlider);

	zoom->addZoomListener(this);
}

ToolZoomSlider::~ToolZoomSlider()
{
	XOJ_RELEASE_TYPE(ToolZoomSlider);
}

void ToolZoomSlider::sliderChanged(GtkRange* range, ToolZoomSlider* self)
{
	XOJ_CHECK_TYPE_OBJ(self, ToolZoomSlider);

	if (self->ignoreChange || !self->sliderChangingByUser)
	{
		return;
	}

	double back = self->zoom->getZoom100() * scaleFuncInv(gtk_range_get_value(range));
	self->zoom->zoomSequnceChange(back, false);
}

bool ToolZoomSlider::sliderButtonPress(GtkRange* range, GdkEvent *event, ToolZoomSlider* self)
{
	XOJ_CHECK_TYPE_OBJ(self, ToolZoomSlider);

	if(!self->sliderChangingByUser)
	{
		self->sliderChangingByUser = true;
		self->zoom->startZoomSequence(-1, -1);
	}
	return false;
}

bool ToolZoomSlider::sliderButtonRelease(GtkRange* range, GdkEvent *event, ToolZoomSlider* self)
{
	XOJ_CHECK_TYPE_OBJ(self, ToolZoomSlider);

	if(self->sliderChangingByUser)
	{
		self->zoom->endZoomSequence();
		self->sliderChangingByUser = false;
	}
	return false;
}

gchar* ToolZoomSlider::sliderFormatValue(GtkRange *range, gdouble value, ToolZoomSlider* self)
{
	return g_strdup_printf("%d%%", (int) (100 * scaleFuncInv(value)));
}

void ToolZoomSlider::zoomChanged()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	if (this->slider == NULL || this->sliderChangingByUser)
	{
		return;
	}

	this->ignoreChange = true;
	double slider_range = scaleFunc(this->zoom->getZoomReal());
	gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
	this->ignoreChange = false;
}

void ToolZoomSlider::zoomRangeValuesChanged()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	updateScaleMarks();
}

string ToolZoomSlider::getToolDisplayName()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	return _("Zoom slider");
}

GtkWidget* ToolZoomSlider::getNewToolIcon()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	return gtk_image_new_from_icon_name("zoom-in" , GTK_ICON_SIZE_SMALL_TOOLBAR);
}

// Should be called when the window size changes
void ToolZoomSlider::updateScaleMarks()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	if (this->slider == NULL)
	{
		return;
	}

	gtk_scale_clear_marks( GTK_SCALE(this->slider));
	gtk_scale_add_mark(GTK_SCALE(this->slider), zoom->getZoom100(),
	                   horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
	gtk_scale_add_mark(GTK_SCALE(this->slider), zoom->getZoomFit(),
	                   horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
}

GtkToolItem* ToolZoomSlider::createItem(bool horizontal)
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	this->horizontal = horizontal;
	this->item = newItem();
	g_object_ref(this->item);

	if (GTK_IS_TOOL_ITEM(this->item))
	{
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(this->item), false);
	}
	if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
	{
		g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
	}
	return this->item;
}

GtkToolItem* ToolZoomSlider::createTmpItem(bool horizontal)
{
	GtkToolItem* item = newItem();
	g_object_ref(item);

	if (GTK_IS_TOOL_ITEM(item))
	{
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
	}

	// no slider marks, but don't matter, because it's only a preview

	gtk_widget_show_all(GTK_WIDGET(item));
	return item;
}

void ToolZoomSlider::enable(bool enabled)
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	if (this->item)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
	if (this->slider)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->slider), enabled);
	}
}

GtkToolItem* ToolZoomSlider::newItem()
{
	XOJ_CHECK_TYPE(ToolZoomSlider);

	GtkToolItem* it = gtk_tool_item_new();

	if (this->slider)
	{
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderChanged), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderButtonPress), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderButtonRelease), this);
	}

	double slider_min = scaleFunc(DEFAULT_ZOOM_MIN);
	double slider_max = scaleFunc(DEFAULT_ZOOM_MAX);
	//slider has 100 steps
	double slider_step = (slider_max - slider_min)/100;

	if (this->horizontal)
	{
		this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
		                                        slider_min,
		                                        slider_max,
		                                        slider_step);
	}
	else
	{
		this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL,
		                                        slider_min,
		                                        slider_max,
		                                        slider_step);
		gtk_range_set_inverted(GTK_RANGE(this->slider), true);
	}

	g_signal_connect(this->slider, "value-changed", G_CALLBACK(sliderChanged), this);
	g_signal_connect(this->slider, "button-press-event", G_CALLBACK(sliderButtonPress), this);
	g_signal_connect(this->slider, "button-release-event", G_CALLBACK(sliderButtonRelease), this);
	g_signal_connect(this->slider, "format-value", G_CALLBACK(sliderFormatValue), this);
	gtk_scale_set_draw_value(GTK_SCALE(this->slider), true);

	if (this->horizontal)
	{
		gtk_widget_set_size_request(GTK_WIDGET(this->slider), 120, 0);
	}
	else
	{
		gtk_widget_set_size_request(GTK_WIDGET(this->slider), 0, 120);
	}

	gtk_container_add(GTK_CONTAINER(it), this->slider);

	ignoreChange = true;
	double slider_range = scaleFunc(this->zoom->getZoomReal());
	gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
	ignoreChange = false;

	updateScaleMarks();

	return it;
}

double ToolZoomSlider::scaleFunc(double x)
{
	return log(x - SCALE_LOG_OFFSET);
}

double ToolZoomSlider::scaleFuncInv(double x)
{
	return exp(x) + SCALE_LOG_OFFSET;
}
