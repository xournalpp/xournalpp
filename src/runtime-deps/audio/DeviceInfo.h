/*
 * Xournal++
 *
 * Class storing information about an audio device
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>  // for string

#include <portaudiocpp/PortAudioCpp.hxx>  // for PaDeviceIndex


class DeviceInfo {
public:
    DeviceInfo(portaudio::Device* device, bool selected);

public:
    inline const std::string& getDeviceName() const { return deviceName; }
    inline PaDeviceIndex getIndex() const { return index; }
    inline bool getSelected() const { return selected; }

private:
    const std::string deviceName;
    const PaDeviceIndex index{};
    const bool selected{};
    const int inputChannels{};
    const int outputChannels{};
};
