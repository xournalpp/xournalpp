/*
 * Xournal++
 *
 * Class to save audio data in an opus file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include <control/settings/Settings.h>

#include "AudioQueue.h"
#include "DeviceInfo.h"

#include <sndfile.h>

#include <thread>
#include <utility>
#include <fstream>

class VorbisConsumer
{
public:
	explicit VorbisConsumer(Settings* settings, AudioQueue<int>* audioQueue);
	~VorbisConsumer();

public:
	bool start(string filename, unsigned int inputChannels);
	void join();
	void stop();

private:
	XOJ_TYPE_ATTRIB;

protected:
	bool stopConsumer = false;

	Settings* settings = nullptr;
	AudioQueue<int>* audioQueue = nullptr;
	std::thread* consumerThread = nullptr;
};
