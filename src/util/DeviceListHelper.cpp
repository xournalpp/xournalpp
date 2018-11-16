#include "DeviceListHelper.h"

#include <i18n.h>


DeviceListHelper::DeviceListHelper(GtkWidget* widget)
{
	GdkDisplay* display;
	if (widget == NULL)
	{
		display = gdk_display_get_default();
	}
	else
	{
		display = gtk_widget_get_display(widget);
	}

	// TODO For never GTK versions, see example here:
	// https://cvs.gnucash.org/docs/MASTER/gnc-cell-renderer-popup_8c_source.html

	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(display);

	addDevicesToList(gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_MASTER));
}

DeviceListHelper::~DeviceListHelper()
{
}

void DeviceListHelper::addDevicesToList(GList* devList)
{
//	gdk_device_get_associated_device();

	// For events: gdk_event_get_source_device();


	while (devList != NULL)
	{
		GdkDevice* dev = (GdkDevice*) devList->data;
		deviceList.push_back(InputDevice(dev));
		addDevicesToList(gdk_device_list_slave_devices(dev));
		devList = devList->next;
	}

	g_list_free(devList);
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
	else if (source == GDK_SOURCE_KEYBOARD)
	{
		return _("keyboard");
	}
	else if (source == GDK_SOURCE_TOUCHSCREEN)
	{
		return _("touchscreen");
	}
	else if (source == GDK_SOURCE_TOUCHPAD)
	{
		return _("touchpad");
	}

	return "";
}
