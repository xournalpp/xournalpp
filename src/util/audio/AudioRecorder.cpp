#include "AudioRecorder.h"

#include <utility>

AudioRecorder::~AudioRecorder() { this->stop(); }

auto AudioRecorder::start(const string& filename) -> bool {
    // Start recording
    bool status = this->portAudioProducer->startRecording();

    // Start the consumer for writing the data
    status = status && this->vorbisConsumer->start(filename);

    return status;
}

void AudioRecorder::stop() {
    // Stop recording audio
    this->portAudioProducer->stopRecording();

    // Wait for libsox to write all the data
    this->vorbisConsumer->join();

    // Reset the queue for the next recording
    this->audioQueue->reset();
}

auto AudioRecorder::isRecording() const -> bool { return this->portAudioProducer->isRecording(); }

auto AudioRecorder::getInputDevices() const -> std::vector<DeviceInfo> {
    return this->portAudioProducer->getInputDevices();
}
