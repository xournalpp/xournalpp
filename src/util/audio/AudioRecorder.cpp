#include <utility>

#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(Settings* settings)
 : settings(settings)
{
    XOJ_INIT_TYPE(AudioRecorder);

    this->audioQueue = new AudioQueue();
    this->portAudioProducer = new PortAudioProducer(settings, this->audioQueue);
    this->soxConsumer = new SoxConsumer(this->audioQueue);
}

AudioRecorder::~AudioRecorder()
{
    XOJ_CHECK_TYPE(AudioRecorder);

    delete this->portAudioProducer;
    this->portAudioProducer = nullptr;

    delete this->soxConsumer;
    this->soxConsumer = nullptr;

    delete this->audioQueue;
    this->audioQueue = nullptr;

    XOJ_RELEASE_TYPE(AudioRecorder);
}

void AudioRecorder::start(std::string filename)
{
    XOJ_CHECK_TYPE(AudioRecorder);

    // Start the consumer for writing the data
    // TODO get sample rate from settings
    this->soxConsumer->start(std::move(filename), 44100.0, this->portAudioProducer->getSelectedInputDevice());

    // Start recording
    this->portAudioProducer->startRecording();
}

void AudioRecorder::stop()
{
    XOJ_CHECK_TYPE(AudioRecorder);

    // Stop recording audio
    this->portAudioProducer->stopRecording();

    // Wait for libsox to write all the data
    this->soxConsumer->join();

    // Reset the queue for the next recording
    this->audioQueue->reset();
}
