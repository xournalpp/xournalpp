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

#include <string>
#include <vector>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

using std::string;
using std::vector;

class InputDevice {
public:
    explicit InputDevice(GdkDevice* device);
    explicit InputDevice(string name, GdkInputSource source);
    ~InputDevice() = default;

public:
    string getType() const;
    string getName() const;
    GdkInputSource getSource() const;
    void updateType(GdkInputSource newSource);

    bool operator==(const InputDevice& inputDevice) const;

private:
    string name;
    GdkInputSource source;
};

namespace DeviceListHelper {
template <typename T>
vector<InputDevice> getDeviceList(T, bool);

vector<InputDevice> getDeviceList(vector<InputDevice>, bool ignoreTouchDevices = false);

template <typename Settings>
vector<InputDevice> getDeviceList(Settings* settings, bool ignoreTouchDevices = false){
    return getDeviceList(settings->getKnownInputDevices(), ignoreTouchDevices);
}

}
