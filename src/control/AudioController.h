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
#include <Path.h>
#include <util/audio/AudioRecorder.h>
#include <util/audio/AudioPlayer.h>
#include <gui/toolbarMenubar/ToolMenuHandler.h>

class AudioController
{
public:
	AudioController(Settings* settings, Control* control);
	virtual ~AudioController();

public:
	bool startRecording();
	bool stopRecording();
	bool isRecording();

	bool isPlaying();
	bool startPlayback(string filename, unsigned int timestamp);
	void pausePlayback();
	void continuePlayback();
	void stopPlayback();
	void seekForwards();
	void seekBackwards();

	string getAudioFilename();
	Path getAudioFolder();
	size_t getStartTime();
	vector<DeviceInfo> getOutputDevices();
	vector<DeviceInfo> getInputDevices();

protected:
	string audioFilename;
	size_t timestamp = 0;
	Settings* settings;
	Control* control;
	AudioRecorder* audioRecorder;
	AudioPlayer* audioPlayer;

private:
	};
