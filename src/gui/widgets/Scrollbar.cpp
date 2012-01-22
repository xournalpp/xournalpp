#include "Scrollbar.h"
#include <math.h>

Scrollbar::Scrollbar(bool horizontal) {
	XOJ_INIT_TYPE(Scrollbar);

#ifdef ENABLE_OS
	if(horizontal) {
		this->scrollbar = os_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
	} else {
		this->scrollbar = os_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
	}
#else
	if (horizontal) {
		this->scrollbar = gtk_hscrollbar_new(NULL);
	} else {
		this->scrollbar = gtk_vscrollbar_new(NULL);
	}
#endif

	g_object_ref(this->scrollbar);

	this->listener = NULL;
	this->adj = gtk_range_get_adjustment(GTK_RANGE(this->scrollbar));

	gtk_adjustment_set_step_increment(this->adj, 20);

	this->value = 50;
	this->setMax(100);
	this->setValue(this->value);

	g_signal_connect(this->adj, "value-changed", G_CALLBACK(scrolled), this);
}

Scrollbar::~Scrollbar() {
	XOJ_RELEASE_TYPE(Scrollbar);

	gtk_widget_unref(this->scrollbar);
	this->scrollbar = NULL;
	g_list_free(this->listener);
	this->listener = NULL;
}

void Scrollbar::scrolled(GtkAdjustment * adjustment, Scrollbar * scrollbar) {
	XOJ_CHECK_TYPE_OBJ(scrollbar, Scrollbar);

	if (scrollbar->value == scrollbar->getValue()) {
		return;
	}

	scrollbar->value = scrollbar->getValue();

	GList * l = scrollbar->listener;
	for (; l != NULL; l = l->next) {
		ScrollbarListener * listener = (ScrollbarListener *) l->data;
		listener->scrolled(scrollbar);
	}
}

double Scrollbar::getWheelDelta(GdkScrollDirection direction) {
	double delta;
	GtkRange * range = GTK_RANGE(this->scrollbar);

	if (GTK_IS_SCROLLBAR(range)) {
		delta = pow(gtk_adjustment_get_page_size(this->adj), 2.0 / 3.0);
	} else {
		delta = gtk_adjustment_get_step_increment(this->adj) * 2;
	}

	if (direction == GDK_SCROLL_UP || direction == GDK_SCROLL_LEFT) {
		delta = -delta;
	}

	if (gtk_range_get_inverted(range)) {
		delta = -delta;
	}

	return delta;
}

int Scrollbar::getValue() {
	XOJ_CHECK_TYPE(Scrollbar);

	return gtk_adjustment_get_value(this->adj);
}

int Scrollbar::getMax() {
	XOJ_CHECK_TYPE(Scrollbar);

	return gtk_adjustment_get_upper(this->adj);
}

void Scrollbar::setMax(int max) {
	XOJ_CHECK_TYPE(Scrollbar);

	gtk_adjustment_set_upper(this->adj, max);

	// Check value within range
	this->setValue(this->value);
}

void Scrollbar::setValue(int value) {
	XOJ_CHECK_TYPE(Scrollbar);

	if (value < gtk_adjustment_get_lower(this->adj)) {
		value = gtk_adjustment_get_lower(this->adj);
	}

	if (value > getMax() - getPageSize()) {
		value = getMax() - getPageSize();
	}

	gtk_adjustment_set_value(this->adj, value);
	this->value = value;

	gtk_widget_set_visible(this->getWidget(), this->getPageSize() < this->getMax());
}

void Scrollbar::scroll(int relPos) {
	XOJ_CHECK_TYPE(Scrollbar);

	this->setValue(this->getValue() + relPos);
}

void Scrollbar::setPageIncrement(int inc) {
	XOJ_CHECK_TYPE(Scrollbar);

	gtk_adjustment_set_page_increment(this->adj, inc);
}

void Scrollbar::ensureAreaIsVisible(int lower, int upper) {
	XOJ_CHECK_TYPE(Scrollbar);

	gtk_adjustment_clamp_page(this->adj, lower, upper);
}

void Scrollbar::setPageSize(int size) {
	XOJ_CHECK_TYPE(Scrollbar);

	gtk_adjustment_set_page_size(this->adj, size);

	// Check value within range
	this->setValue(this->value);
}

int Scrollbar::getPageSize() {
	XOJ_CHECK_TYPE(Scrollbar);

	return gtk_adjustment_get_page_size(this->adj);
}

GtkWidget * Scrollbar::getWidget() {
	XOJ_CHECK_TYPE(Scrollbar);

	return this->scrollbar;
}

void Scrollbar::addListener(ScrollbarListener * listener) {
	XOJ_CHECK_TYPE(Scrollbar);

	this->listener = g_list_prepend(this->listener, listener);
}

void Scrollbar::removeScrollbarListener(ScrollbarListener * listener) {
	XOJ_CHECK_TYPE(Scrollbar);

	this->listener = g_list_remove(this->listener, listener);
}

