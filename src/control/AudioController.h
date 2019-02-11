#pragma once

#include <XournalType.h>
#include "settings/Settings.h"
#include "Control.h"
#include <Path.h>
#include <util/audio/AudioRecorder.h>
#include <util/audio/AudioPlayer.h>

class AudioController
{
public:
	AudioController(Settings* settings, Control* control);
	virtual ~AudioController();

public:
	bool isRecording();
	void recToggle();
	void recStartStop(bool record);
	string getAudioFilename();
	Path getAudioFolder();
	size_t getStartTime();
	AudioRecorder* getAudioRecorder();
	AudioPlayer* getAudioPlayer();

protected:
	string audioFilename;
	size_t sttime = 0;
	Settings* settings;
	Control* control;
	AudioRecorder* audioRecorder;
	AudioPlayer* audioPlayer;

private:
	XOJ_TYPE_ATTRIB;

};
