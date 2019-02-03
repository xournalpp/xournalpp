/*
 * Xournal++
 *
 * Class to save audio data in an mp3 file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "AudioQueue.h"
#include "DeviceInfo.h"

#include <sox.h>

#include <thread>
#include <utility>

class SoxConsumer
{
public:
	explicit SoxConsumer(AudioQueue *audioQueue);
	~SoxConsumer();

public:
	void start(std::string filename, double sampleRate, const DeviceInfo& inputDevice);
	void join();
	void stop();

private:
	XOJ_TYPE_ATTRIB;

protected:
	sox_signalinfo_t* inputSignal = nullptr;
	sox_format_t* outputFile = nullptr;
	bool stopConsumer = false;

	AudioQueue* audioQueue;
	std::thread* consumerThread = nullptr;
};


