#include "DeviceListHelper.h"

#include <i18n.h>


DeviceListHelper::DeviceListHelper(GtkWidget* widget)
{
#if GTK3_ENABLED
	GdkDisplay* display;
	if (widget == NULL)
	{
		widget = gdk_display_get_default();
	}
	else
	{
		display = gtk_widget_get_display(widget);
	}

	deviceManager = gdk_display_get_device_manager(display);
	this->cbDevice = gtk_combo_box_text_new();
	GList* devList = gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_SLAVE);
#else
	GList* devList = gdk_devices_list();
#endif

	while (devList != NULL)
	{
		deviceList.push_back(InputDevice((GdkDevice*) devList->data));
		devList = devList->next;
	}
}

DeviceListHelper::~DeviceListHelper()
{
}

std::vector<InputDevice>& DeviceListHelper::getDeviceList()
{
	return deviceList;
}



InputDevice::InputDevice(GdkDevice* device)
 : device(device)
{

}

InputDevice::~InputDevice()
{
}

GdkDevice* InputDevice::getDevice()
{
	return device;
}

string InputDevice::getType()
{
	if (device->source == GDK_SOURCE_MOUSE)
	{
		return _("mouse");
	}
	else if (device->source == GDK_SOURCE_PEN)
	{
		return _("pen");
	}
	else if (device->source == GDK_SOURCE_ERASER)
	{
		return _("eraser");
	}
	else if (device->source == GDK_SOURCE_CURSOR)
	{
		return _("cursor");
	}

	return "";
}
