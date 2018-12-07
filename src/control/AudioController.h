#pragma once

#include <XournalType.h>
#include "settings/Settings.h"
#include "Control.h"
#include <string>
using std::string;

class AudioController
{
public:
	AudioController(Settings* settings, Control* control);
	virtual ~AudioController();

	bool isRecording();
	void recToggle();
	void recStartStop(bool record);
	string getAudioFilename();
	string getAudioFolder();
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
