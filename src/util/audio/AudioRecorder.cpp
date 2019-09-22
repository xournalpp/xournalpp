#include <utility>

#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(Settings* settings)
		: settings(settings)
{
	this->audioQueue = new AudioQueue<float>();
	this->portAudioProducer = new PortAudioProducer(settings, this->audioQueue);
	this->vorbisConsumer = new VorbisConsumer(settings, this->audioQueue);
}

AudioRecorder::~AudioRecorder()
{
	this->stop();

	delete this->portAudioProducer;
	this->portAudioProducer = nullptr;

	delete this->vorbisConsumer;
	this->vorbisConsumer = nullptr;

	delete this->audioQueue;
	this->audioQueue = nullptr;
}

bool AudioRecorder::start(string filename)
{
	// Start recording
	bool status = this->portAudioProducer->startRecording();

	// Start the consumer for writing the data
	status = status && this->vorbisConsumer->start(std::move(filename));

	return status;
}

void AudioRecorder::stop()
{
	// Stop recording audio
	this->portAudioProducer->stopRecording();

	// Wait for libsox to write all the data
	this->vorbisConsumer->join();

	// Reset the queue for the next recording
	this->audioQueue->reset();
}

bool AudioRecorder::isRecording()
{
	return this->portAudioProducer->isRecording();
}

std::vector<DeviceInfo> AudioRecorder::getInputDevices()
{
	std::list<DeviceInfo> deviceList = this->portAudioProducer->getInputDevices();
	return vector<DeviceInfo>{std::make_move_iterator(std::begin(deviceList)),
							  std::make_move_iterator(std::end(deviceList))};
}
