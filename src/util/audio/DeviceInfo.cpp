#include "DeviceInfo.h"

DeviceInfo::DeviceInfo(portaudio::Device* device, bool selected):
        deviceName(device->name()),
        index(device->index()),
        selected(selected),
        inputChannels((device->isFullDuplexDevice() || device->isInputOnlyDevice()) ? device->maxInputChannels() : 0),
        outputChannels((device->isFullDuplexDevice() || device->isOutputOnlyDevice()) ? device->maxOutputChannels() :
                                                                                        0) {}

auto DeviceInfo::getDeviceName() const -> const string& { return deviceName; }

auto DeviceInfo::getIndex() const -> PaDeviceIndex { return index; }

auto DeviceInfo::getSelected() const -> bool { return selected; }
