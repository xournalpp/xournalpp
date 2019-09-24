#include "SpinPageAdapter.h"

SpinPageAdapter::SpinPageAdapter()
{
	this->lastTimeoutId = 0;
	this->widget = gtk_spin_button_new_with_range(0, 0, 1);
	g_object_ref(this->widget);

	g_signal_connect(this->widget, "value-changed", G_CALLBACK(pageNrSpinChangedCallback), this);

	this->page = -1;
}

SpinPageAdapter::~SpinPageAdapter()
{
	g_object_unref(this->widget);
	this->widget = nullptr;
}

bool SpinPageAdapter::pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter)
{
	adapter->lastTimeoutId = 0;
	adapter->page = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(adapter->widget));

	adapter->firePageChanged();
	return false;
}

void SpinPageAdapter::pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter)
{
	// Nothing changed.
	if (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton)) == (long) adapter->page)
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
	return this->widget;
}

int SpinPageAdapter::getPage()
{
	return this->page;
}

void SpinPageAdapter::setPage(size_t page)
{
	this->page = page;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(this->widget), page);
}

void SpinPageAdapter::setMinMaxPage(size_t min, size_t max)
{
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(this->widget), min, max);
}

void SpinPageAdapter::addListener(SpinPageListener* listener)
{
	this->listener.push_back(listener);
}

void SpinPageAdapter::removeListener(SpinPageListener* listener)
{
	this->listener.remove(listener);
}

void SpinPageAdapter::firePageChanged()
{
	for (SpinPageListener* listener : this->listener)
	{
		listener->pageChanged(this->page);
	}
}

SpinPageListener::~SpinPageListener() { }

