/*
 * Xournal++
 *
 * Class to record audio and store it as MP3-file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "AudioQueue.h"
#include "PortAudioProducer.h"
#include "SoxConsumer.h"

#include <control/settings/Settings.h>

class AudioRecorder
{
public:
	explicit AudioRecorder(Settings* settings);
	~AudioRecorder();

public:
	void start(std::string filename);
	void stop();

private:
	XOJ_TYPE_ATTRIB;

protected:
	Settings* settings;

	AudioQueue* audioQueue;
	PortAudioProducer* portAudioProducer;
	SoxConsumer* soxConsumer;
};


