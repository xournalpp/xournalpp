#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(Settings* settings) : settings(settings)
{
	XOJ_INIT_TYPE(AudioPlayer);

	this->audioQueue = new AudioQueue<float>();
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

void AudioPlayer::start(string filename, unsigned int timestamp)
{
	XOJ_CHECK_TYPE(AudioPlayer);

	// Start the producer for reading the data
	this->vorbisProducer->start(std::move(filename), this->portAudioConsumer->getSelectedOutputDevice(), timestamp);
	vorbis_info* vi = this->vorbisProducer->getSignalInformation();

	// Start playing
	this->portAudioConsumer->startPlaying(static_cast<double>(vi->rate), static_cast<unsigned int>(vi->channels));

	// Clean up after audio is played
	stopThread = std::thread([&]
							 {
								 while (this->portAudioConsumer->isPlaying())
								 {
									 Pa_Sleep(100);
								 }
								 this->stop();
							 });
	//TODO we need something nicer here then detaching the thread
	stopThread.detach();
}

void AudioPlayer::stop()
{
	XOJ_CHECK_TYPE(AudioPlayer);

	// Wait for libsox to read all the data
	this->vorbisProducer->stop();

	// Stop playing audio
	this->portAudioConsumer->stopPlaying();

	// Reset the queue for the next playback
	this->audioQueue->reset();
}

void AudioPlayer::abort()
{
	XOJ_CHECK_TYPE(AudioPlayer);

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
