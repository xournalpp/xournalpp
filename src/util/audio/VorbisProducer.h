/*
 * Xournal++
 *
 * Class to read audio data from an mp3 file
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

#include <sndfile.h>

#include <thread>
#include <utility>

class VorbisProducer
{
public:
	explicit VorbisProducer(AudioQueue<int>* audioQueue);
	~VorbisProducer();

public:
	void start(string filename, const DeviceInfo& outputDevice, unsigned int timestamp);
	SF_INFO* getSignalInformation();
	void abort();
	void stop();

private:
	XOJ_TYPE_ATTRIB;

protected:
	bool stopProducer = false;
	SF_INFO sfInfo;
	SNDFILE_tag* sfFile = nullptr;

	AudioQueue<int>* audioQueue = nullptr;
	std::thread* producerThread = nullptr;
};
