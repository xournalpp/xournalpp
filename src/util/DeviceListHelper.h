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

class InputDevice
{
public:
	InputDevice(GdkDevice* device);
	virtual ~InputDevice();

public:
	GdkDevice* getDevice();
	string getType();

private:
	GdkDevice* device;
};


class DeviceListHelper
{
public:
	DeviceListHelper(GtkWidget* widget);
	virtual ~DeviceListHelper();

public:
	std::vector<InputDevice>& getDeviceList();

private:
	std::vector<InputDevice> deviceList;
};
