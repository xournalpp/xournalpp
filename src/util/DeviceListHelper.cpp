#include "DeviceListHelper.h"

#include <i18n.h>


DeviceListHelper::DeviceListHelper()
{
	// For never GTK versions, see example here:
	// https://cvs.gnucash.org/docs/MASTER/gnc-cell-renderer-popup_8c_source.html

	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(gdk_display_get_default());

	addDevicesToList(gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_SLAVE));

	if (deviceList.size() == 0)
	{
		g_warning("No device found. Is Xournal++ running in debugger / Eclipse...?\nProbably this is the reason for not finding devices!\n");
	}
}

DeviceListHelper::~DeviceListHelper()
{
}

void DeviceListHelper::addDevicesToList(GList* devList)
{
	while (devList != NULL)
	{
		GdkDevice* dev = (GdkDevice*) devList->data;
		if (GDK_SOURCE_KEYBOARD == gdk_device_get_source(dev))
		{
			// Skip keyboard
			devList = devList->next;
			continue;
		}

		deviceList.push_back(InputDevice(dev));
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
		// not used: filtered above
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
