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
    this->portAudioProducer->stopRecording();
    this->vorbisConsumer->join();  // libsox must write all the data before we can continue
    this->audioQueue->reset();     // next recording requires empty queue
}

auto AudioRecorder::isRecording() const -> bool { return this->portAudioProducer->isRecording(); }

auto AudioRecorder::getInputDevices() const -> std::vector<DeviceInfo> {
    return this->portAudioProducer->getInputDevices();
}
