/*
 * Xournal++
 *
 * Class to record audio using libportaudio
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "DeviceInfo.h"
#include "AudioQueue.h"

#include <control/settings/Settings.h>

#include <list>

#include <portaudiocpp/PortAudioCpp.hxx>

class PortAudioProducer
{
public:
	explicit PortAudioProducer(Settings* settings, AudioQueue<float>* audioQueue);
	~PortAudioProducer();

	std::list<DeviceInfo> getInputDevices();

	const DeviceInfo getSelectedInputDevice();

	bool isRecording();

	bool startRecording();

	int recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

	void stopRecording();

private:
	protected:
	const unsigned long framesPerBuffer = 64;

	portaudio::AutoSystem autoSys;
	portaudio::System& sys;
	Settings* settings;
	AudioQueue<float>* audioQueue;

	int inputChannels = 0;

	portaudio::MemFunCallbackStream<PortAudioProducer>* inputStream = nullptr;
};
