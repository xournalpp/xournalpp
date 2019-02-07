#include "DeviceInfo.h"

DeviceInfo::DeviceInfo(portaudio::Device* device, bool selected)
		: deviceName(device->name()),
		  index(device->index()),
		  selected(selected),
		  inputChannels((device->isFullDuplexDevice() || device->isInputOnlyDevice()) ? device->maxInputChannels() : 0),
		  outputChannels((device->isFullDuplexDevice() || device->isOutputOnlyDevice()) ? device->maxOutputChannels() : 0)
{
	XOJ_INIT_TYPE(DeviceInfo);
}

DeviceInfo::DeviceInfo(const DeviceInfo& other)
		: deviceName(other.deviceName),
		  index(other.index),
		  selected(other.selected),
		  inputChannels(other.inputChannels),
		  outputChannels(other.outputChannels)
{
	XOJ_INIT_TYPE(DeviceInfo);
}

DeviceInfo::~DeviceInfo()
{
	XOJ_CHECK_TYPE(DeviceInfo);

	XOJ_RELEASE_TYPE(DeviceInfo);
}

const string& DeviceInfo::getDeviceName() const
{
	XOJ_CHECK_TYPE(DeviceInfo);

	return deviceName;
}

const PaDeviceIndex DeviceInfo::getIndex() const
{
	XOJ_CHECK_TYPE(DeviceInfo);

	return index;
}

const bool DeviceInfo::getSelected() const
{
	XOJ_CHECK_TYPE(DeviceInfo);

	return selected;
}

const int DeviceInfo::getInputChannels() const
{
	XOJ_CHECK_TYPE(DeviceInfo);

	return inputChannels;
}

const int DeviceInfo::getOutputChannels() const
{
	return outputChannels;
}
