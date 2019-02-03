#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(Settings *settings) : settings(settings)
{
    XOJ_INIT_TYPE(AudioPlayer);

    this->audioQueue = new AudioQueue();
    this->portAudioConsumer = new PortAudioConsumer(settings, this->audioQueue);
    this->soxProducer = new SoxProducer(this->audioQueue);
}

AudioPlayer::~AudioPlayer()
{
    XOJ_CHECK_TYPE(AudioPlayer);

    delete this->portAudioConsumer;
    this->portAudioConsumer = nullptr;

    delete this->soxProducer;
    this->soxProducer = nullptr;

    delete this->audioQueue;
    this->audioQueue = nullptr;

    XOJ_RELEASE_TYPE(AudioPlayer);
}

void AudioPlayer::start(string filename, unsigned int timestamp)
{
    XOJ_CHECK_TYPE(AudioPlayer);

    // Start the producer for reading the data
    this->soxProducer->start(std::move(filename), this->portAudioConsumer->getSelectedOutputDevice(), timestamp);
    sox_signalinfo_t *signal = this->soxProducer->getSignalInformation();

    // Start playing
    if (signal->rate > 1)
    {
        this->portAudioConsumer->startPlaying(signal->rate, signal->channels);
    }
    else
    {
        this->portAudioConsumer->startPlaying(44100.0, signal->channels);
    }

    // Clean up after audio is played
    stopThread = std::thread([&] {
        while(this->portAudioConsumer->isPlaying())
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
    this->soxProducer->stop();

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
    this->soxProducer->abort();

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
