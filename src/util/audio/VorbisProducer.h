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

#include <vorbis/vorbisfile.h>

#include <thread>
#include <utility>
#include <fstream>

class VorbisProducer
{
public:
	explicit VorbisProducer(AudioQueue<float>* audioQueue);
	~VorbisProducer();

public:
	void start(string filename, const DeviceInfo& outputDevice, unsigned int timestamp);
	vorbis_info* getSignalInformation();
	void abort();
	void stop();

	static size_t read(void* buffer, size_t elementSize, size_t elementCount, void* dataSource);
	static int seek(void* dataSource, ogg_int64_t offset, int origin);
	static long tell(void* dataSource);

private:
	XOJ_TYPE_ATTRIB;

protected:
protected:
	bool stopProducer = false;
	std::ifstream inputFile;
	OggVorbis_File vf;
	vorbis_info* vi = nullptr;

	AudioQueue<float>* audioQueue = nullptr;
	std::thread* producerThread = nullptr;
};


