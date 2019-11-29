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

#include "config.h"
#include <XournalType.h>
#include "settings/Settings.h"
#include "Control.h"
#include "../util/Path.h"
#include <gui/toolbarMenubar/ToolMenuHandler.h>

#ifndef ENABLE_AUDIO
class DeviceInfo {};
#else
#include "util/audio/DeviceInfo.h"
#endif

/**
 * Dummy audio controller used if audio feature is disabled globally
 * Actual implementation is in class AudioControllerImpl
 */
class AudioController
{
public:
	AudioController(Settings* settings, Control* control);
	virtual ~AudioController();

public:
	virtual bool startRecording();
	virtual bool stopRecording();
	virtual bool isRecording();

	virtual bool isPlaying();
	virtual bool startPlayback(string filename, unsigned int timestamp);
	virtual void pausePlayback();
	virtual void continuePlayback();
	virtual void stopPlayback();
	virtual void seekForwards();
	virtual void seekBackwards();

	virtual string getAudioFilename();
	virtual Path getAudioFolder();
	virtual size_t getStartTime() const;
	virtual vector<DeviceInfo> getOutputDevices();
	virtual vector<DeviceInfo> getInputDevices();

protected:
	string audioFilename;
	size_t timestamp = 0;
	Settings* settings;
	Control* control;
};
