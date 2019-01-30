#include "DeviceInfo.h"

DeviceInfo::DeviceInfo(portaudio::Device *device, bool selected) : deviceName(device->name()),
                                                    index(device->index()),
                                                    selected(selected),
                                                    inputChannels((device->isFullDuplexDevice() || device->isInputOnlyDevice()) ? device->maxInputChannels() : 0),
                                                    outputChannels((device->isFullDuplexDevice() || device->isOutputOnlyDevice()) ? device->maxOutputChannels() : 0)
{
    XOJ_INIT_TYPE(DeviceInfo);
}

DeviceInfo::~DeviceInfo()
{
    XOJ_CHECK_TYPE(DeviceInfo);

    XOJ_RELEASE_TYPE(DeviceInfo);
}
