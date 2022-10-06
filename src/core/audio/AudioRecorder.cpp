#include "AudioRecorder.h"

#include "audio/AudioQueue.h"         // for AudioQueue
#include "audio/DeviceInfo.h"         // for DeviceInfo
#include "audio/PortAudioProducer.h"  // for PortAudioProducer
#include "audio/VorbisConsumer.h"     // for VorbisConsumer

AudioRecorder::AudioRecorder(Settings& settings):
        audioQueue(std::make_unique<AudioQueue<float>>()),
        portAudioProducer(std::make_unique<PortAudioProducer>(settings, *audioQueue)),
        vorbisConsumer(std::make_unique<VorbisConsumer>(settings, *audioQueue)) {}

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
