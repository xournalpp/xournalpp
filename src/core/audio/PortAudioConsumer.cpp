#include "PortAudioConsumer.h"

#include <algorithm>  // for for_each, transform, max
#include <deque>      // for _Deque_iterator
#include <iterator>   // for next, prev
#include <string>     // for to_string, string

#include <glib.h>  // for g_warning

#include "audio/AudioQueue.h"           // for AudioQueue
#include "audio/DeviceInfo.h"           // for DeviceInfo
#include "control/settings/Settings.h"  // for Settings

#include "AudioPlayer.h"  // for AudioPlayer

constexpr auto FRAMES_PER_BUFFER{64U};

auto PortAudioConsumer::getOutputDevices() const -> std::vector<DeviceInfo> {
    std::vector<DeviceInfo> deviceList;
    deviceList.reserve(this->sys.deviceCount());

    for (auto i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i) {
        if (i->isFullDuplexDevice() || i->isOutputOnlyDevice()) {
            DeviceInfo deviceInfo(&(*i), this->audioPlayer.getSettings().getAudioOutputDevice() == i->index());
            deviceList.push_back(deviceInfo);
        }
    }
    return deviceList;
}

auto PortAudioConsumer::getSelectedOutputDevice() const -> DeviceInfo {
    try {
        return DeviceInfo(&sys.deviceByIndex(this->audioPlayer.getSettings().getAudioOutputDevice()), true);
    } catch (portaudio::PaException& e) {
        g_warning("PortAudioConsumer: Selected output device was not found - fallback to default output device\nCaused "
                  "by: %s",
                  e.what());
        return DeviceInfo(&sys.defaultOutputDevice(), true);
    }
}

auto PortAudioConsumer::isPlaying() const -> bool { return this->outputStream && this->outputStream->isActive(); }

auto PortAudioConsumer::startPlaying() -> bool {
    // Abort a playback stream if one is currently active
    if (isPlaying()) {
        this->outputStream->abort();
    }

    auto [sampleRate, channels] = this->audioQueue.getAudioAttributes();

    if (sampleRate == -1) {
        g_warning("PortAudioConsumer: Timing issue - Sample rate requested before known");
        return false;
    }

    // Get the device information of our output device
    portaudio::Device* device = nullptr;
    try {
        device = &sys.deviceByIndex(getSelectedOutputDevice().getIndex());
    } catch (portaudio::PaException& e) {
        g_warning("PortAudioConsumer: Unable to find selected output device");
        return false;
    }

    if (static_cast<unsigned int>(device->maxOutputChannels()) < channels) {
        this->audioQueue.signalEndOfStream();
        g_warning("Output device has not enough channels to play audio file. (Requires at least 2 channels)");
        return false;
    }

    this->outputChannels = channels;
    portaudio::DirectionSpecificStreamParameters outParams(*device, channels, portaudio::FLOAT32, true,
                                                           device->defaultLowOutputLatency(), nullptr);
    portaudio::StreamParameters params(portaudio::DirectionSpecificStreamParameters::null(), outParams, sampleRate,
                                       FRAMES_PER_BUFFER, paNoFlag);

    try {
        this->outputStream = std::make_unique<portaudio::MemFunCallbackStream<PortAudioConsumer>>(
                params, *this, &PortAudioConsumer::playCallback);
    } catch (portaudio::PaException& e) {
        this->audioQueue.signalEndOfStream();
        g_warning("PortAudioConsumer: Unable to open stream to device\nCaused by: %s", e.what());
        return false;
    }
    // Start the recording
    try {
        this->outputStream->start();
    } catch (portaudio::PaException& e) {
        this->audioQueue.signalEndOfStream();
        g_warning("PortAudioConsumer: Unable to start stream\nCaused by: %s", e.what());
        this->outputStream.reset();
        return false;
    }
    return true;
}

auto PortAudioConsumer::playCallback(const void* /*inputBuffer*/, void* outputBuffer, unsigned long framesPerBuffer,
                                     const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags statusFlags) -> int {
    if (statusFlags) {
        g_warning("PortAudioConsumer: PortAudio reported a stream warning: %s", std::to_string(statusFlags).c_str());
    }

    if (outputBuffer != nullptr) {
        auto begI = static_cast<float*>(outputBuffer);
        auto midI = this->audioQueue.pop(begI, framesPerBuffer * this->outputChannels);
        auto endI = std::next(begI, framesPerBuffer * this->outputChannels);
        // Fill buffer to requested length if necessary

        if (midI != endI) {
            // Show underflow warning if there are not enough samples and the stream is not yet finished
            if (!this->audioQueue.hasStreamEnded()) {
                g_warning("PortAudioConsumer: Not enough audio samples available to fill requested frame");
            }

            if (midI > std::next(begI, this->outputChannels)) {
                // If there is previous audio data use this data to ramp down the audio samples
                std::transform(std::prev(midI, this->outputChannels), std::prev(endI, this->outputChannels), midI,
                               [](auto&& ampl) { return ampl / 2.0; });
            } else {
                // If there is no data that could be used to ramp down just output silence
                std::for_each(midI, endI, [](auto& ampl) { ampl = 0; });
            }
        }

        // Continue playback if there is still data available
        if (this->audioQueue.hasStreamEnded() && this->audioQueue.empty()) {
            this->audioPlayer.disableAudioPlaybackButtons();
            return paComplete;
        }


        return paContinue;
    }

    // The output buffer is no longer available - Abort!
    this->audioQueue.signalEndOfStream();
    this->audioPlayer.disableAudioPlaybackButtons();
    return paAbort;
}

void PortAudioConsumer::stopPlaying() {
    // Stop the playback
    if (this->outputStream) {
        try {
            if (this->outputStream->isActive()) {
                this->outputStream->stop();
            }
        } catch (portaudio::PaException& e) {
            /*
             * We try closing the stream but this->outputStream might be an invalid object at this time if the stream
             * was previously closed by the backend. Just ignore this as the stream is closed either way.
             */
        }
    }
    this->outputStream.reset();
}
