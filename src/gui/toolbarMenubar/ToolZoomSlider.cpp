#include "ToolZoomSlider.h"
// TODO: AA: type check

ToolZoomSlider::ToolZoomSlider(ActionHandler * handler, String id, ActionType type, ZoomControl * zoom) :
	AbstractToolItem(id, handler, type, NULL) {
	slider = NULL;
	this->zoom = zoom;

	this->horizontal = true;
	this->fixed = NULL;

	zoom->addZoomListener(this);
}

ToolZoomSlider::~ToolZoomSlider() {
}

void ToolZoomSlider::sliderChanged(GtkRange * range, ZoomControl * zoom) {
	zoom->setZoom(gtk_range_get_value(range));
}

void ToolZoomSlider::zoomChanged(double lastZoom) {
	gtk_range_set_value(GTK_RANGE(slider), zoom->getZoom());
}

void ToolZoomSlider::zoomRangeValuesChanged() {
	updateScaleMarks();
}

// Should be called when the window size changes
void ToolZoomSlider::updateScaleMarks() {
	if (!slider) {
		return;
	}
	gtk_scale_clear_marks( GTK_SCALE(slider));
	gtk_scale_add_mark(GTK_SCALE(slider), zoom->getZoom100(), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
	gtk_scale_add_mark(GTK_SCALE(slider), zoom->getZoomFit(), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
}

void ToolZoomSlider::setHorizontal(bool horizontal) {
	if (horizontal) {
		GtkAllocation alloc = { 0, 0, 120, 30 };
		gtk_widget_set_allocation(fixed, &alloc);
		gtk_widget_set_size_request(slider, 120, 25);
	} else {
		GtkAllocation alloc = { 0, 0, 30, 120 };
		gtk_widget_set_allocation(fixed, &alloc);
		gtk_widget_set_size_request(slider, 25, 120);
	}
	updateScaleMarks();
}

GtkToolItem * ToolZoomSlider::createItem(bool horizontal) {
	this->horizontal = horizontal;
	GtkToolItem * item = newItem();
	g_object_ref(item);

	if (GTK_IS_TOOL_ITEM(item)) {
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
	}
	if (GTK_IS_TOOL_BUTTON(item) || GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
		g_signal_connect(item, "clicked", G_CALLBACK(&toolButtonCallback), this);
	}
	gtk_object_ref(GTK_OBJECT(item));
	return item;

}

void ToolZoomSlider::enable(bool enabled) {
	if (this->item) {
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
	if (this->slider) {
		gtk_widget_set_sensitive(GTK_WIDGET(this->slider), enabled);
	}
}

GtkToolItem * ToolZoomSlider::newItem() {
	GtkToolItem * it = gtk_tool_item_new();

	if (slider) {
		g_signal_handlers_disconnect_by_func(slider, (void*) (sliderChanged), zoom);
	}

	if (horizontal) {
		slider = gtk_hscale_new_with_range(0.3, 3, 0.1);
	} else {
		slider = gtk_vscale_new_with_range(0.3, 3, 0.1);
		gtk_range_set_inverted(GTK_RANGE(slider), true);
	}
	g_signal_connect(slider, "value-changed", G_CALLBACK(sliderChanged), zoom);
	gtk_scale_set_draw_value(GTK_SCALE(slider), false);

	fixed = gtk_fixed_new();

	GtkAllocation alloc = { 0, 0, 125, 30 };
	if (!horizontal) {
		alloc.width = 30;
		alloc.height = 125;
	}

	gtk_widget_set_allocation(fixed, &alloc);

	if (horizontal) {
		gtk_fixed_put(GTK_FIXED(fixed), slider, 0, 5);
		gtk_widget_set_size_request(slider, 120, 25);
	} else {
		gtk_fixed_put(GTK_FIXED(fixed), slider, 5, 0);
		gtk_widget_set_size_request(slider, 25, 120);
	}

	gtk_container_add(GTK_CONTAINER(it), fixed);
	gtk_range_set_value(GTK_RANGE(slider), zoom->getZoom());
	updateScaleMarks();

	return it;
}
