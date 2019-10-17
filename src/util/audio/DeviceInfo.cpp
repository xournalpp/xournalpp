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
		: deviceName(other.deviceName),
		  index(other.index),
		  selected(other.selected),
		  inputChannels(other.inputChannels),
		  outputChannels(other.outputChannels)
{
}

DeviceInfo::~DeviceInfo()
{
}

const string& DeviceInfo::getDeviceName() const
{
	return deviceName;
}

const PaDeviceIndex DeviceInfo::getIndex() const
{
	return index;
}

const bool DeviceInfo::getSelected() const
{
	return selected;
}

const int DeviceInfo::getInputChannels() const
{
	return inputChannels;
}

const int DeviceInfo::getOutputChannels() const
{
	return outputChannels;
}
