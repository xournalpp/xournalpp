#include "AbstractInputDevice.h"

AbstractInputDevice::AbstractInputDevice(GtkWidget* widget, XournalView* view)
 : widget(widget),
   view(view)
{
	XOJ_INIT_TYPE(AbstractInputDevice);
}

AbstractInputDevice::~AbstractInputDevice()
{
	XOJ_CHECK_TYPE(AbstractInputDevice);

	widget = NULL;
	view = NULL;

	XOJ_RELEASE_TYPE(AbstractInputDevice);
}
