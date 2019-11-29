#include "AudioController.h"

AudioController::AudioController(Settings* settings, Control* control)
{
	this->settings = settings;
	this->control = control;
}

AudioController::~AudioController()
{
}

auto AudioController::startRecording() -> bool
{
	return false;
}

auto AudioController::stopRecording() -> bool
{
	return true;
}

auto AudioController::isRecording() -> bool
{
	return false;
}

auto AudioController::isPlaying() -> bool
{
	return false;
}

auto AudioController::startPlayback(string filename, unsigned int timestamp) -> bool
{
	return false;
}

void AudioController::pausePlayback()
{

}

void AudioController::seekForwards()
{

}

void AudioController::seekBackwards()
{

}

void AudioController::continuePlayback()
{

}

void AudioController::stopPlayback()
{

}

auto AudioController::getAudioFilename() -> string
{
	return "";
}

auto AudioController::getAudioFolder() -> Path
{
	return Path::fromUri("/");
}

auto AudioController::getStartTime() const -> size_t
{
	return 0;
}

auto AudioController::getOutputDevices() -> vector<DeviceInfo>
{
	return vector<DeviceInfo>();
}

auto AudioController::getInputDevices() -> vector<DeviceInfo>
{
	return vector<DeviceInfo>();
}
