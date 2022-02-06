#include "AudioRecorder.h"

#include <utility>

AudioRecorder::~AudioRecorder() { this->stop(); }

auto AudioRecorder::start(fs::path const& file) -> bool {
    bool status = this->portAudioProducer->startRecording();
    // Start the consumer for writing the data
    status = status && this->vorbisConsumer->start(file);
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
