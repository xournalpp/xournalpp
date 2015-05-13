#include "SpinPageAdapter.h"

SpinPageAdapter::SpinPageAdapter()
{
	XOJ_INIT_TYPE(SpinPageAdapter);

	this->lastTimeoutId = 0;
	this->widget = gtk_spin_button_new_with_range(0, 0, 1);
	g_object_ref(this->widget);

	g_signal_connect(this->widget, "value-changed", G_CALLBACK(pageNrSpinChangedCallback), this);

	this->page = -1;
}

SpinPageAdapter::~SpinPageAdapter()
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	g_object_unref(this->widget);
	this->widget = NULL;

	XOJ_RELEASE_TYPE(SpinPageAdapter);
}

bool SpinPageAdapter::pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, SpinPageAdapter);
	adapter->lastTimeoutId = 0;
	adapter->page = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(adapter->widget));

	gdk_threads_enter();
	adapter->firePageChanged();
	gdk_threads_leave();
	return false;
}

void SpinPageAdapter::pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, SpinPageAdapter);

	// Nothing changed.
	if (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton)) == adapter->page)
	{
		return;
	}

	if (adapter->lastTimeoutId)
	{
		g_source_remove(adapter->lastTimeoutId);
	}

	// Give the spin button some time to realease, if we don't do he will send new events...
	adapter->lastTimeoutId = g_timeout_add(100, (GSourceFunc) pageNrSpinChangedTimerCallback, adapter);
}

GtkWidget* SpinPageAdapter::getWidget()
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	return this->widget;
}

int SpinPageAdapter::getPage()
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	return this->page;
}

void SpinPageAdapter::setPage(int page)
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	this->page = page;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(this->widget), page);
}

void SpinPageAdapter::setMinMaxPage(int min, int max)
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(this->widget), min, max);
}

void SpinPageAdapter::addListener(SpinPageListener* listener)
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	this->listener.push_back(listener);
}

void SpinPageAdapter::removeListener(SpinPageListener* listener)
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	this->listener.remove(listener);
}

void SpinPageAdapter::firePageChanged()
{
	XOJ_CHECK_TYPE(SpinPageAdapter);

	for (SpinPageListener* listener : this->listener)
	{
		listener->pageChanged(this->page);
	}
}
