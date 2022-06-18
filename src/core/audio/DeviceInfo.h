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
    const std::string& getDeviceName() const;
    PaDeviceIndex getIndex() const;
    bool getSelected() const;

private:
    const std::string deviceName;
    const PaDeviceIndex index{};
    const bool selected{};
    const int inputChannels{};
    const int outputChannels{};
};
