#include "AudioController.h"
#include "Util.h"

#include <iostream>
#include <i18n.h>
#include <XojMsgBox.h>

AudioController::AudioController(Settings* settings, Control* control)
{
	XOJ_INIT_TYPE(AudioController);
	this->settings = settings;
	this->control = control;
}

AudioController::~AudioController()
{
	XOJ_CHECK_TYPE(AudioController);

	XOJ_RELEASE_TYPE(AudioController);
}

bool AudioController::isRecording()
{
	XOJ_CHECK_TYPE(AudioController);

	return this->recording;
}

void AudioController::recStartStop(bool rec)
{
	XOJ_CHECK_TYPE(AudioController);

	string command;

	if (rec)
	{
		if (getAudioFolder() == "")
		{
			return;
		}

		this->recording = true;
		sttime = (g_get_monotonic_time() / 1000000);

		char buffer[50];
		time_t secs = time(0);
		tm *t = localtime(&secs);
		//This prints the date and time in ISO format.
		sprintf(buffer, "%04d-%02d-%02d_%02d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour,
				t->tm_min, t->tm_sec);
		string data(buffer);
		data += ".mp3";

		audioFilename = data;

		printf("Start recording\n");
		command = "xopp-recording.sh start " + getAudioFolder() + "/" + data;
		//std::cout<<"COMMAND: "<<command<<"\n";
	}
	else if (this->recording)
	{
		this->recording = false;
		audioFilename = "";
		sttime = 0;
		command = "xopp-recording.sh stop";
	}
	system(command.c_str());
}

void AudioController::recToggle()
{
	XOJ_CHECK_TYPE(AudioController);

	if (!this->recording)
	{
		recStartStop(true);
	}
	else
	{
		recStartStop(false);
	}

}

string AudioController::getAudioFilename()
{
	XOJ_CHECK_TYPE(AudioController);

	return this->audioFilename;
}

string AudioController::getAudioFolder()
{
	XOJ_CHECK_TYPE(AudioController);

	string af = this->settings->getAudioFolder();

	if (af.length() < 8)
	{
		string msg = _("Audio folder not set! Recording won't work!\nPlease set the "
					   "recording folder under \"Preferences > Audio recording\"");
		g_warning("%s", msg.c_str());
		XojMsgBox::showErrorToUser(this->control->getGtkWindow(), msg);
		return "";
	}

	af.erase(af.begin(), af.begin() + 7);
	return af;
}

gint AudioController::getStartTime()
{
	XOJ_CHECK_TYPE(AudioController);

	return this->sttime;
}
