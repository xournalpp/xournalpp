#include <utility>

#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(Settings* settings)
		: settings(settings)
{
	XOJ_INIT_TYPE(AudioRecorder);

	this->audioQueue = new AudioQueue<int>();
	this->portAudioProducer = new PortAudioProducer(settings, this->audioQueue);
	this->vorbisConsumer = new VorbisConsumer(settings, this->audioQueue);
}

AudioRecorder::~AudioRecorder()
{
	XOJ_CHECK_TYPE(AudioRecorder);

	delete this->portAudioProducer;
	this->portAudioProducer = nullptr;

	delete this->vorbisConsumer;
	this->vorbisConsumer = nullptr;

	delete this->audioQueue;
	this->audioQueue = nullptr;

	XOJ_RELEASE_TYPE(AudioRecorder);
}

bool AudioRecorder::start(string filename)
{
	XOJ_CHECK_TYPE(AudioRecorder);

	// Start the consumer for writing the data
	bool status = this->vorbisConsumer->start(std::move(filename), static_cast<unsigned int>(this->portAudioProducer->getSelectedInputDevice().getInputChannels()));

	// Start recording
	status &= this->portAudioProducer->startRecording();

	return status;
}

void AudioRecorder::stop()
{
	XOJ_CHECK_TYPE(AudioRecorder);

	// Stop recording audio
	this->portAudioProducer->stopRecording();

	// Wait for libsox to write all the data
	this->vorbisConsumer->join();

	// Reset the queue for the next recording
	this->audioQueue->reset();
}

bool AudioRecorder::isRecording()
{
	XOJ_CHECK_TYPE(AudioRecorder);

	return this->portAudioProducer->isRecording();
}

std::vector<DeviceInfo> AudioRecorder::getInputDevices()
{
	XOJ_CHECK_TYPE(AudioRecorder);

	std::list<DeviceInfo> deviceList = this->portAudioProducer->getInputDevices();
	return vector<DeviceInfo>{std::make_move_iterator(std::begin(deviceList)),
							  std::make_move_iterator(std::end(deviceList))};
}
