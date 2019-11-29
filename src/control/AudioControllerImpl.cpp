#include "AudioControllerImpl.h"
#include "Util.h"

#include <i18n.h>
#include <XojMsgBox.h>

AudioControllerImpl::AudioControllerImpl(Settings* settings, Control* control) : AudioController(settings, control)
{
	this->settings = settings;
	this->control = control;
	this->audioRecorder = new AudioRecorder(settings);
	this->audioPlayer = new AudioPlayer(control, settings);
}

AudioControllerImpl::~AudioControllerImpl()
{
	delete this->audioRecorder;
	this->audioRecorder = nullptr;

	delete this->audioPlayer;
	this->audioPlayer = nullptr;
}

auto AudioControllerImpl::startRecording() -> bool
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

auto AudioControllerImpl::stopRecording() -> bool
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

auto AudioControllerImpl::isRecording() -> bool
{
	return this->audioRecorder->isRecording();
}

auto AudioControllerImpl::isPlaying() -> bool
{
	return this->audioPlayer->isPlaying();
}

auto AudioControllerImpl::startPlayback(string filename, unsigned int timestamp) -> bool
{
	this->audioPlayer->stop();
	bool status = this->audioPlayer->start(filename, timestamp);
	if (status)
	{
		this->control->getWindow()->getToolMenuHandler()->enableAudioPlaybackButtons();
	}
	return status;
}

void AudioControllerImpl::pausePlayback()
{
	this->control->getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(true);

	this->audioPlayer->pause();
}

void AudioControllerImpl::seekForwards()
{
	this->audioPlayer->seek(this->settings->getDefaultSeekTime());
}

void AudioControllerImpl::seekBackwards()
{
	this->audioPlayer->seek(-1 * this->settings->getDefaultSeekTime());
}

void AudioControllerImpl::continuePlayback()
{
	this->control->getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(false);

	this->audioPlayer->play();
}

void AudioControllerImpl::stopPlayback()
{
	this->control->getWindow()->getToolMenuHandler()->disableAudioPlaybackButtons();
	this->audioPlayer->stop();
}

auto AudioControllerImpl::getAudioFilename() -> string
{
	return this->audioFilename;
}

auto AudioControllerImpl::getAudioFolder() -> Path
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

auto AudioControllerImpl::getStartTime() const -> size_t
{
	return this->timestamp;
}

auto AudioControllerImpl::getOutputDevices() -> vector<DeviceInfo>
{
	return this->audioPlayer->getOutputDevices();
}

auto AudioControllerImpl::getInputDevices() -> vector<DeviceInfo>
{
	return this->audioRecorder->getInputDevices();
}
