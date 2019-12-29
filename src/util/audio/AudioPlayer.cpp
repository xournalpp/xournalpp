#include "AudioPlayer.h"

#include "control/Control.h"

AudioPlayer::~AudioPlayer() { this->stop(); }

auto AudioPlayer::start(const string& filename, unsigned int timestamp) -> bool {
    // Start the producer for reading the data
    bool status = this->vorbisProducer->start(filename, timestamp);

    // Start playing
    if (status) {
        status = status && this->play();
    }

    return status;
}

auto AudioPlayer::isPlaying() -> bool { return this->portAudioConsumer->isPlaying(); }

void AudioPlayer::pause() {
    if (!this->portAudioConsumer->isPlaying()) {
        return;
    }

    // Stop playing audio
    this->portAudioConsumer->stopPlaying();
}

auto AudioPlayer::play() -> bool {
    if (this->portAudioConsumer->isPlaying()) {
        return false;
    }

    return this->portAudioConsumer->startPlaying();
}

void AudioPlayer::disableAudioPlaybackButtons() {
    if (this->audioQueue->hasStreamEnded()) {
        this->control.getWindow()->disableAudioPlaybackButtons();
    }
}

void AudioPlayer::stop() {
    // Stop playing audio
    this->portAudioConsumer->stopPlaying();

    this->audioQueue->signalEndOfStream();

    // Abort libsox
    this->vorbisProducer->abort();

    // Reset the queue for the next playback
    this->audioQueue->reset();
}

void AudioPlayer::seek(int seconds) {
    // set seek flag here in vorbisProducer
    this->vorbisProducer->seek(seconds);
}

auto AudioPlayer::getOutputDevices() -> vector<DeviceInfo> { return this->portAudioConsumer->getOutputDevices(); }

auto AudioPlayer::getSettings() -> Settings& { return this->settings; }
