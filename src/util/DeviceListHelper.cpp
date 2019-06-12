#include <utility>

#include "DeviceListHelper.h"

#include <i18n.h>
#include <algorithm>


DeviceListHelper::DeviceListHelper(Settings* settings, bool ignoreTouchDevices)
 : ignoreTouchDevices(ignoreTouchDevices)
{
	deviceList = settings->getKnownInputDevices();
	// TODO ignore touchscreens

#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 20)
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* defaultSeat = gdk_display_get_default_seat(display);
	GdkDevice* pointer = gdk_seat_get_pointer(defaultSeat);
	GdkSeat* pointerSeat = gdk_device_get_seat(pointer);
	GList* pointerSlaves = gdk_seat_get_slaves(pointerSeat, GDK_SEAT_CAPABILITY_ALL_POINTING);
	addDevicesToList(pointerSlaves);
	g_list_free(pointerSlaves);
#else
	GdkDeviceManager* deviceManager = gdk_display_get_device_manager(gdk_display_get_default());

	addDevicesToList(gdk_device_manager_list_devices(deviceManager, GDK_DEVICE_TYPE_SLAVE));
#endif

	if (deviceList.empty())
	{
		g_warning("No device found. Is Xournal++ running in debugger / Eclipse...?\nProbably this is the reason for not finding devices!\n");
	}
}

DeviceListHelper::~DeviceListHelper()
{
}

void DeviceListHelper::addDevicesToList(GList* devList)
{
	while (devList != nullptr)
	{
		auto dev = (GdkDevice*) devList->data;
		if (GDK_SOURCE_KEYBOARD == gdk_device_get_source(dev))
		{
			// Skip keyboard
			devList = devList->next;
			continue;
		}
		if (gdk_device_get_vendor_id(dev) == nullptr && gdk_device_get_product_id(dev) == nullptr)
		{
			// Skip core pointer
			devList = devList->next;
			continue;
		}
		if (ignoreTouchDevices && GDK_SOURCE_TOUCHSCREEN == gdk_device_get_source(dev))
		{
			devList = devList->next;
			continue;
		}

		this->storeNewUnlistedDevice(dev);
		devList = devList->next;
	}
}

void DeviceListHelper::storeNewUnlistedDevice(GdkDevice* device)
{
	// This could potentially be problematic with systems having a multitude of input devices as it searches linearily
	auto it = std::find(deviceList.begin(), deviceList.end(), InputDevice(device));
	if (it != deviceList.end())
	{
		// Device is already known but source may be unknown
		it->updateType(gdk_device_get_source(device));
		return;
	}

	deviceList.emplace_back(device);

}

std::vector<InputDevice>& DeviceListHelper::getDeviceList()
{
	return deviceList;
}

InputDevice::InputDevice(GdkDevice* device) : name(gdk_device_get_name(device)), source(gdk_device_get_source(device))
{

}

InputDevice::InputDevice(string name, GdkInputSource source) : name(std::move(name)), source(source)
{

}

InputDevice::~InputDevice() = default;

string InputDevice::getName() const
{
	return name;
}

void InputDevice::updateType(GdkInputSource newSource)
{
	this->source = newSource;
}

string InputDevice::getType() const
{
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

GdkInputSource InputDevice::getSource() const
{
	return this->source;
}

bool InputDevice::operator==(const InputDevice& inputDevice) const
{
	return this->getName() == inputDevice.getName();
}
