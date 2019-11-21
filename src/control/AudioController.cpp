#include "AudioController.h"
#include "Util.h"

#include <i18n.h>
#include <XojMsgBox.h>

AudioController::AudioController(Settings* settings, Control* control)
{
	this->settings = settings;
	this->control = control;
	this->audioRecorder = new AudioRecorder(settings);
	this->audioPlayer = new AudioPlayer(control, settings);
}

AudioController::~AudioController()
{
	delete this->audioRecorder;
	this->audioRecorder = nullptr;

	delete this->audioPlayer;
	this->audioPlayer = nullptr;
}

auto AudioController::startRecording() -> bool
{
	if (!this->isRecording())
	{
		if (getAudioFolder().isEmpty())
		{
			return false;
		}

		this->timestamp = static_cast<size_t>(g_get_monotonic_time() / 1000);

		std::array<char, 50> buffer{};
		time_t secs = time(nullptr);
		tm* t = localtime(&secs);
		// This prints the date and time in ISO format.
		sprintf(buffer.data(), "%04d-%02d-%02d_%02d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		        t->tm_hour, t->tm_min, t->tm_sec);
		string data(buffer.data());
		data += ".ogg";

		audioFilename = data;

		g_message("Start recording");

		bool isRecording = this->audioRecorder->start(getAudioFolder().str() + "/" + data);

		if (!isRecording)
		{
			audioFilename = "";
			this->timestamp = 0;
		}

		return isRecording;
	}
	return false;
}

auto AudioController::stopRecording() -> bool
{
	if (this->audioRecorder->isRecording())
	{
		audioFilename = "";
		this->timestamp = 0;

		g_message("Stop recording");

		this->audioRecorder->stop();
	}
	return true;
}

auto AudioController::isRecording() -> bool
{
	return this->audioRecorder->isRecording();
}

auto AudioController::isPlaying() -> bool
{
	return this->audioPlayer->isPlaying();
}

auto AudioController::startPlayback(const string& filename, unsigned int timestamp) -> bool
{
	this->audioPlayer->stop();
	bool status = this->audioPlayer->start(filename, timestamp);
	if (status)
	{
		this->control->getWindow()->getToolMenuHandler()->enableAudioPlaybackButtons();
	}
	return status;
}

void AudioController::pausePlayback()
{
	this->control->getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(true);

	this->audioPlayer->pause();
}

void AudioController::seekForwards()
{
	this->audioPlayer->seek(this->settings->getDefaultSeekTime());
}

void AudioController::seekBackwards()
{
	this->audioPlayer->seek(-1 * this->settings->getDefaultSeekTime());
}

void AudioController::continuePlayback()
{
	this->control->getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(false);

	this->audioPlayer->play();
}

void AudioController::stopPlayback()
{
	this->control->getWindow()->getToolMenuHandler()->disableAudioPlaybackButtons();
	this->audioPlayer->stop();
}

auto AudioController::getAudioFilename() -> string
{
	return this->audioFilename;
}

auto AudioController::getAudioFolder() -> Path
{
	string af = this->settings->getAudioFolder();

	if (af.length() < 8)
	{
		string msg = _("Audio folder not set! Recording won't work!\nPlease set the "
					   "recording folder under \"Preferences > Audio recording\"");
		g_warning("%s", msg.c_str());
		XojMsgBox::showErrorToUser(this->control->getGtkWindow(), msg);
		return Path("");
	}

	return Path::fromUri(af);
}

auto AudioController::getStartTime() -> size_t
{
	return this->timestamp;
}

auto AudioController::getOutputDevices() -> vector<DeviceInfo>
{
	return this->audioPlayer->getOutputDevices();
}

auto AudioController::getInputDevices() -> vector<DeviceInfo>
{
	return this->audioRecorder->getInputDevices();
}
