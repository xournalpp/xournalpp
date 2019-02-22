#include <control/Control.h>
#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(Control* control, Settings* settings) : control(control), settings(settings)
{
	XOJ_INIT_TYPE(AudioPlayer);

	this->audioQueue = new AudioQueue<int>();
	this->portAudioConsumer = new PortAudioConsumer(settings, this->audioQueue);
	this->vorbisProducer = new VorbisProducer(this->audioQueue);
}

AudioPlayer::~AudioPlayer()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	delete this->portAudioConsumer;
	this->portAudioConsumer = nullptr;

	delete this->vorbisProducer;
	this->vorbisProducer = nullptr;

	delete this->audioQueue;
	this->audioQueue = nullptr;

	XOJ_RELEASE_TYPE(AudioPlayer);
}

bool AudioPlayer::start(string filename, unsigned int timestamp)
{
	XOJ_CHECK_TYPE(AudioPlayer);

	// Start the producer for reading the data
	bool status = this->vorbisProducer->start(std::move(filename), timestamp);

	// Start playing
	if (status)
	{
		status &= this->play();
	}

	return status;
}

bool AudioPlayer::isPlaying()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	return this->portAudioConsumer->isPlaying();
}

void AudioPlayer::pause()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	if (!this->portAudioConsumer->isPlaying())
	{
		return;
	}

	// Stop playing audio
	this->portAudioConsumer->stopPlaying();
}

bool AudioPlayer::play()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	if (this->portAudioConsumer->isPlaying())
	{
		return false;
	}

	bool status = this->portAudioConsumer->startPlaying();

	if (status)
	{
		// Clean up after audio is played
		stopThread = std::thread(
				[&]
				{
					while (isPlaying())
					{
						Pa_Sleep(100);
					}
					this->portAudioConsumer->stopPlaying();

					// If the stream is played completely update the UI elements accordingly
					if (this->audioQueue->hasStreamEnded())
					{
						this->control->getWindow()->disableAudioPlaybackButtons();
					}
				});
		stopThread.detach();
	}

	return status;
}

void AudioPlayer::stop()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	this->audioQueue->signalEndOfStream();

	// Stop playing audio
	this->portAudioConsumer->stopPlaying();

	// Abort libsox
	this->vorbisProducer->abort();

	// Reset the queue for the next playback
	this->audioQueue->reset();
}

vector<DeviceInfo> AudioPlayer::getOutputDevices()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	std::list<DeviceInfo> deviceList = this->portAudioConsumer->getOutputDevices();
	return vector<DeviceInfo>{std::make_move_iterator(std::begin(deviceList)),
							  std::make_move_iterator(std::end(deviceList))};
}
