#include "DeviceListHelper.h"

#include <i18n.h>


DeviceListHelper::DeviceListHelper(GtkWidget* widget)
{
#if GTK3_ENABLED
	GdkDisplay* display;
	if (widget == NULL)
	{
		display = gdk_display_get_default();
	}
	else
	{
		display = gtk_widget_get_display(widget);
	}

	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(display);
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

string InputDevice::getName()
{
	return gdk_device_get_name(device);
}

string InputDevice::getType()
{
	GdkInputSource source = gdk_device_get_source(device);
	if (source == GDK_SOURCE_MOUSE)
	{
		return _("mouse");
	}
	else if (source == GDK_SOURCE_PEN)
	{
		return _("pen");
	}
	else if (source == GDK_SOURCE_ERASER)
	{
		return _("eraser");
	}
	else if (source == GDK_SOURCE_CURSOR)
	{
		return _("cursor");
	}

	return "";
}
