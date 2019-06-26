/*
 * Xournal++
 *
 * Helper functions to iterate over devices
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */


#pragma once

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <vector>
#include <string>

using std::string;
using std::vector;

class InputDevice
{
public:
	explicit InputDevice(GdkDevice* device);
	~InputDevice() = default;

public:
	GdkDevice* getDevice() const;
	string getType() const;
	string getName() const;

private:
	GdkDevice* device;
};


class DeviceListHelper
{
public:
	DeviceListHelper(bool ignoreTouchDevices = false);
	virtual ~DeviceListHelper();

public:
	vector<InputDevice>& getDeviceList();

private:
	void addDevicesToList(GList* devList);

private:
	bool ignoreTouchDevices;

	vector<InputDevice> deviceList;
};
