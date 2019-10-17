#include "ToolZoomSlider.h"

#include "control/zoom/ZoomControl.h"

#include <config.h>
#include <i18n.h>
#include <StringUtils.h>
#include <cmath>

ToolZoomSlider::ToolZoomSlider(ActionHandler* handler, string id, ActionType type, ZoomControl* zoom)
 : AbstractToolItem(id, handler, type, nullptr),
   zoom(zoom)
{
	zoom->addZoomListener(this);
}

ToolZoomSlider::~ToolZoomSlider()
{
}

void ToolZoomSlider::sliderChanged(GtkRange* range, ToolZoomSlider* self)
{
	if (!self->sliderChangingByZoomControlOrInit &&
		!self->zoom->isZoomPresentationMode() &&
		(self->sliderChangingBySliderDrag || self->sliderChangingBySliderHoverScroll))
	{
		double back = self->zoom->getZoom100Value() * scaleFuncInv(gtk_range_get_value(range));
		self->zoom->zoomSequenceChange(back, false);
	}
	self->sliderChangingBySliderHoverScroll = false;
}

bool ToolZoomSlider::sliderButtonPress(GtkRange* range, GdkEvent *event, ToolZoomSlider* self)
{
	if(!self->sliderChangingBySliderDrag && !self->zoom->isZoomPresentationMode())
	{
		self->zoom->setZoomFitMode(false);
		self->zoom->startZoomSequence(-1, -1);
		self->sliderChangingBySliderDrag = true;
	}
	return false;
}

bool ToolZoomSlider::sliderButtonRelease(GtkRange* range, GdkEvent *event, ToolZoomSlider* self)
{
	self->zoom->endZoomSequence();
	self->sliderChangingBySliderDrag = false;
	return false;
}

bool ToolZoomSlider::sliderHoverScroll(GtkWidget* range,  GdkEventScroll* event, ToolZoomSlider* self)
{
	gint64 now = g_get_monotonic_time();
	if (now > self->sliderHoverScrollLastTime + 500)
	{
		self->zoom->setZoomFitMode(false);
		self->zoom->startZoomSequence(-1, -1);
	}
	self->sliderChangingBySliderHoverScroll = true;
	self->sliderHoverScrollLastTime = now;
	return false;
}

gchar* ToolZoomSlider::sliderFormatValue(GtkRange *range, gdouble value, ToolZoomSlider* self)
{
	return g_strdup_printf("%d%%", (int) (100 * scaleFuncInv(value)));
}

void ToolZoomSlider::zoomChanged()
{
	if (this->slider == nullptr || this->sliderChangingBySliderDrag)
	{
		return;
	}

	this->sliderChangingByZoomControlOrInit = true;
	double slider_range = scaleFunc(this->zoom->getZoomReal());
	gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
	this->sliderChangingByZoomControlOrInit = false;
}

void ToolZoomSlider::zoomRangeValuesChanged()
{
	updateScaleMarks();
}

string ToolZoomSlider::getToolDisplayName()
{
	return _("Zoom slider");
}

GtkWidget* ToolZoomSlider::getNewToolIcon()
{
	return gtk_image_new_from_icon_name("zoom-in" , GTK_ICON_SIZE_SMALL_TOOLBAR);
}

// Should be called when the window size changes
void ToolZoomSlider::updateScaleMarks()
{
	if (this->slider == nullptr)
	{
		return;
	}

	gtk_scale_clear_marks(GTK_SCALE(this->slider));
	gtk_scale_add_mark(GTK_SCALE(this->slider), scaleFunc(zoom->getZoom100Value()), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
	gtk_scale_add_mark(GTK_SCALE(this->slider), scaleFunc(zoom->getZoomFitValue()), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
}

GtkToolItem* ToolZoomSlider::createItem(bool horizontal)
{
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
	GtkToolItem* it = gtk_tool_item_new();

	if (this->slider)
	{
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderChanged), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderButtonPress), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderButtonRelease), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderHoverScroll), this);
		g_signal_handlers_disconnect_by_func(this->slider, (void* )(sliderFormatValue), this);
	}

	double sliderMin = scaleFunc(DEFAULT_ZOOM_MIN);
	double sliderMax = scaleFunc(DEFAULT_ZOOM_MAX);
	// slider has 100 steps
	double sliderStep = (sliderMax - sliderMin) / 100;

	if (this->horizontal)
	{
		this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, sliderMin, sliderMax, sliderStep);
	}
	else
	{
		this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, sliderMin, sliderMax, sliderStep);
		gtk_range_set_inverted(GTK_RANGE(this->slider), true);
	}

	g_signal_connect(this->slider, "value-changed", G_CALLBACK(sliderChanged), this);
	g_signal_connect(this->slider, "button-press-event", G_CALLBACK(sliderButtonPress), this);
	g_signal_connect(this->slider, "button-release-event", G_CALLBACK(sliderButtonRelease), this);
	g_signal_connect(this->slider, "scroll-event", G_CALLBACK(sliderHoverScroll), this);
	g_signal_connect(this->slider, "format-value", G_CALLBACK(sliderFormatValue), this);
	gtk_scale_set_draw_value(GTK_SCALE(this->slider), true);

	gtk_widget_set_can_focus(this->slider, false);
	
	if (this->horizontal)
	{
		gtk_widget_set_size_request(GTK_WIDGET(this->slider), 120, 16);
	}
	else
	{
		gtk_widget_set_size_request(GTK_WIDGET(this->slider), 16, 120);
	}

	gtk_container_add(GTK_CONTAINER(it), this->slider);

	sliderChangingByZoomControlOrInit = true;
	double slider_range = scaleFunc(this->zoom->getZoomReal());
	gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
	sliderChangingByZoomControlOrInit = false;

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
