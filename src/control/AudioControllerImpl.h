/*
 * Xournal++
 *
 * Audio Recording / Playing controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include "settings/Settings.h"
#include "Control.h"
#include "AudioController.h"
#include <Path.h>
#include <util/audio/AudioRecorder.h>
#include <util/audio/AudioPlayer.h>
#include <gui/toolbarMenubar/ToolMenuHandler.h>

class AudioControllerImpl: public AudioController
{
public:
	AudioControllerImpl(Settings* settings, Control* control);
	~AudioControllerImpl() override;

public:
	bool startRecording() override;
	bool stopRecording() override;
	bool isRecording() override;

	bool isPlaying() override;
	bool startPlayback(string filename, unsigned int timestamp) override;
	void pausePlayback() override;
	void continuePlayback() override;
	void stopPlayback() override;
	void seekForwards() override;
	void seekBackwards() override;

	string getAudioFilename() override;
	Path getAudioFolder() override;
	size_t getStartTime() const override;
	vector<DeviceInfo> getOutputDevices() override;
	vector<DeviceInfo> getInputDevices() override;

protected:
	string audioFilename;
	size_t timestamp = 0;
	Settings* settings;
	Control* control;
	AudioRecorder* audioRecorder;
	AudioPlayer* audioPlayer;

private:
};
