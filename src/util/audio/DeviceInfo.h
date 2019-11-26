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

#include <XournalType.h>

#include <portaudiocpp/PortAudioCpp.hxx>
#include <string>

class DeviceInfo
{
public:
	DeviceInfo(portaudio::Device* device, bool selected);

public:
	const string& getDeviceName() const;
	PaDeviceIndex getIndex() const;
	bool getSelected() const;
	int getInputChannels() const;
	int getOutputChannels() const;

private:
	const string deviceName;
	const PaDeviceIndex index{};
	const bool selected{};
	const int inputChannels{};
	const int outputChannels{};
};
