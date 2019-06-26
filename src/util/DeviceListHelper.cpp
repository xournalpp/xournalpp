#include "DeviceListHelper.h"

#include <i18n.h>


DeviceListHelper::DeviceListHelper(bool ignoreTouchDevices)
 : ignoreTouchDevices(ignoreTouchDevices)
{
#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 20)
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* defaultSeat = gdk_display_get_default_seat(display);
	GdkDevice* pointer = gdk_seat_get_pointer(defaultSeat);
	GdkSeat* pointerSeat = gdk_device_get_seat(pointer);
	GList* pointerSlaves = gdk_seat_get_slaves(pointerSeat, GDK_SEAT_CAPABILITY_ALL_POINTING);
	addDevicesToList(pointerSlaves);
#else
	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(gdk_display_get_default());

	addDevicesToList(gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_SLAVE));
#endif

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
		if (ignoreTouchDevices && GDK_SOURCE_TOUCHSCREEN == gdk_device_get_source(dev))
		{
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

{

}

InputDevice::InputDevice(GdkDevice* device) : device(device)
{

}

GdkDevice* InputDevice::getDevice() const
{
	return device;
}

string InputDevice::getName() const
{
	return gdk_device_get_name(device);
}

string InputDevice::getType() const
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
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
	else if (source == GDK_SOURCE_TRACKPOINT)
	{
		return _("trackpoint");
	}
#endif

	return "";
}
