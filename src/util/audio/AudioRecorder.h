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
#include "VorbisConsumer.h"

#include <control/settings/Settings.h>

class AudioRecorder
{
public:
	explicit AudioRecorder(Settings* settings);
	~AudioRecorder();

public:
	bool start(string filename);
	void stop();
	bool isRecording();
	vector<DeviceInfo> getInputDevices();

private:
	protected:
	Settings* settings = nullptr;

	AudioQueue<float>* audioQueue = nullptr;
	PortAudioProducer* portAudioProducer = nullptr;
	VorbisConsumer* vorbisConsumer = nullptr;
};
