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
	explicit VorbisConsumer(Settings* settings, AudioQueue<float>* audioQueue);
	~VorbisConsumer();

public:
	bool start(string filename);
	void join();
	void stop();

private:
	protected:
	bool stopConsumer = false;

	Settings* settings = nullptr;
	AudioQueue<float>* audioQueue = nullptr;
	std::thread* consumerThread = nullptr;
};
