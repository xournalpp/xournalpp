#include "DeviceListHelper.h"

#include <i18n.h>

#include <vector>

std::vector<InputDevice> addDevicesToList(GList* const devList, bool ignoreTouchDevices)
{
	std::vector<InputDevice> v;
	for (auto iter = devList; iter != nullptr; iter = iter->next)
	{
		auto* dev = (GdkDevice*) iter->data;
		if (GDK_SOURCE_KEYBOARD == gdk_device_get_source(dev))
		{
			continue;
		}
		if (ignoreTouchDevices && GDK_SOURCE_TOUCHSCREEN == gdk_device_get_source(dev))
		{
			continue;
		}

		v.emplace_back(dev);
	}

	g_list_free(devList);
	return v;
}


vector<InputDevice> DeviceListHelper::getDeviceList(bool ignoreTouchDevices)
{
	vector<InputDevice> devices;

	GList* pointerSlaves;
	// TODO remove after completely switching to gtk 3.20 or use c++17 if constexpr (predicate){...} else{...} ...
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* defaultSeat = gdk_display_get_default_seat(display);
	GdkDevice* pointer = gdk_seat_get_pointer(defaultSeat);
	GdkSeat* pointerSeat = gdk_device_get_seat(pointer);
	pointerSlaves = gdk_seat_get_slaves(pointerSeat, GDK_SEAT_CAPABILITY_ALL_POINTING);
#else
	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(gdk_display_get_default());
	pointerSlaves = gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_SLAVE);
#endif
	devices = addDevicesToList(pointerSlaves, ignoreTouchDevices);

	if (devices.empty())
	{
		g_warning("No device found. Is Xournal++ running in debugger / Eclipse...?\n"
		          "Probably this is the reason for not finding devices!\n");
	}
	return devices;
}

InputDevice::InputDevice(GdkDevice* device)
 : device(device)
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
