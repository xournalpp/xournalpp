#pragma once

#include <XournalType.h>
#include "settings/Settings.h"
#include "Control.h"
#include <Path.h>

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
	gint getStartTime();

protected:
	bool recording = false;
	string audioFilename;
	gint sttime = 0;	
	Settings* settings;
	Control* control;

private:
	XOJ_TYPE_ATTRIB;

};
