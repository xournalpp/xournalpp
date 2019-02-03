/*
 * Xournal++
 *
 * Class to play audio from a MP3-file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "AudioQueue.h"
#include "PortAudioConsumer.h"
#include "SoxProducer.h"

#include <control/settings/Settings.h>

class AudioPlayer
{
public:
	explicit AudioPlayer(Settings* settings);
	~AudioPlayer();
	void start(string filename, unsigned int timestamp = 0);
	void stop();
	void abort();

	vector<DeviceInfo> getOutputDevices();

private:
	XOJ_TYPE_ATTRIB;

protected:
	Settings* settings = nullptr;

	AudioQueue* audioQueue = nullptr;
	PortAudioConsumer* portAudioConsumer = nullptr;
	SoxProducer* soxProducer = nullptr;
	std::thread stopThread;
};


