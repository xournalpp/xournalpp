#include "DeviceInfo.h"

DeviceInfo::DeviceInfo(portaudio::Device* device, bool selected)
		: deviceName(device->name()),
		  index(device->index()),
		  selected(selected),
		  inputChannels((device->isFullDuplexDevice() || device->isInputOnlyDevice()) ? device->maxInputChannels() : 0),
		  outputChannels((device->isFullDuplexDevice() || device->isOutputOnlyDevice()) ? device->maxOutputChannels() : 0)
{
}

DeviceInfo::DeviceInfo(const DeviceInfo& other)


        = default;

DeviceInfo::~DeviceInfo() = default;

auto DeviceInfo::getDeviceName() const -> const string&
{
	return deviceName;
}

auto DeviceInfo::getIndex() const -> const PaDeviceIndex
{
	return index;
}

auto DeviceInfo::getSelected() const -> const bool
{
	return selected;
}

auto DeviceInfo::getInputChannels() const -> const int
{
	return inputChannels;
}

auto DeviceInfo::getOutputChannels() const -> const int
{
	return outputChannels;
}
