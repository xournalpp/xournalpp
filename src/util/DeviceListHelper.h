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

namespace DeviceListHelper
{
vector<InputDevice> getDeviceList(bool ignoreTouchDevices = false);
}
