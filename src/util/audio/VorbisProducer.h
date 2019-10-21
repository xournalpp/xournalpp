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
	explicit VorbisProducer(AudioQueue<float>* audioQueue);
	~VorbisProducer();

public:
	bool start(string filename, unsigned int timestamp);
	void abort();
	void stop();
	void seek(int seconds);

private:
	const int sample_buffer_size = 16384;
	protected:
	bool stopProducer = false;
	SF_INFO sfInfo;
	SNDFILE_tag* sfFile = nullptr;

	AudioQueue<float>* audioQueue = nullptr;
	std::thread* producerThread = nullptr;

	int seekSeconds = 0;
};
