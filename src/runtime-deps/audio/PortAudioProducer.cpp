#include "PortAudioProducer.h"

#include <algorithm>  // for min, max
#include <cstddef>    // for size_t
#include <iterator>   // for next
#include <string>     // for to_string, string

#include <glib.h>  // for g_message

#include "audio/AudioQueue.h"     // for AudioQueue
#include "audio/AudioSettings.h"  // for AudioSettings
#include "audio/DeviceInfo.h"     // for DeviceInfo
#include "util/safe_casts.h"      // for as_unsigned


constexpr auto FRAMES_PER_BUFFER{64U};

auto PortAudioProducer::getInputDevices() const -> std::vector<DeviceInfo> {
    auto devCount = as_unsigned(this->sys.deviceCount());
    std::vector<DeviceInfo> deviceList;
    deviceList.reserve(devCount);

    for (auto i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i) {

        if (i->isFullDuplexDevice() || i->isInputOnlyDevice()) {
            DeviceInfo deviceInfo(&(*i), this->settings.audioInputDevice == i->index());
            deviceList.push_back(deviceInfo);
        }
    }
    return deviceList;
}

auto PortAudioProducer::getSelectedInputDevice() const -> DeviceInfo {
    try {
        return DeviceInfo(&sys.deviceByIndex(this->settings.audioInputDevice), true);
    } catch (const portaudio::PaException& e) {
        g_message(
                "PortAudioProducer: Selected input device not found - fallback to default input device\nCaused by: %s",
                e.what());
        return DeviceInfo(&sys.defaultInputDevice(), true);
    }
}

auto PortAudioProducer::isRecording() const -> bool { return this->inputStream && this->inputStream->isActive(); }

auto PortAudioProducer::startRecording() -> bool {
    // Check if there already is a recording
    if (this->inputStream) {
        return false;
    }

    // Get the device information of our input device
    portaudio::Device* device = nullptr;
    try {
        device = &sys.deviceByIndex(getSelectedInputDevice().getIndex());
    } catch (const portaudio::PaException& e) {
        g_message("PortAudioProducer: Unable to find selected input device: %s", e.what());
        return false;
    }

    // Restrict recording channels to 2 as playback devices should have 2 channels at least
    this->inputChannels = std::min(2, device->maxInputChannels());
    portaudio::DirectionSpecificStreamParameters inParams(*device, this->inputChannels, portaudio::FLOAT32, true,
                                                          device->defaultLowInputLatency(), nullptr);
    portaudio::StreamParameters params(inParams, portaudio::DirectionSpecificStreamParameters::null(),
                                       this->settings.audioSampleRate, FRAMES_PER_BUFFER, paNoFlag);

    this->audioQueue.setAudioAttributes(this->settings.audioSampleRate, static_cast<unsigned int>(this->inputChannels));

    // Specify the callback used for buffering the recorded data
    try {
        this->inputStream = std::make_unique<portaudio::MemFunCallbackStream<PortAudioProducer>>(
                params, *this, &PortAudioProducer::recordCallback);
    } catch (const portaudio::PaException& e) {
        g_message("PortAudioProducer: Unable to open stream: %s", e.what());
        return false;
    }

    // Start the recording
    try {
        this->inputStream->start();
    } catch (const portaudio::PaException& e) {
        g_message("PortAudioProducer: Unable to start stream: %s", e.what());
        this->inputStream.reset();
        return false;
    }

    return true;
}

auto PortAudioProducer::recordCallback(const void* inputBuffer, void* /*outputBuffer*/, unsigned long framesPerBuffer,
                                       const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags statusFlags) -> int {
    if (statusFlags) {
        g_message("PortAudioProducer: statusFlag: %s", std::to_string(statusFlags).c_str());
    }

    if (inputBuffer != nullptr) {
        size_t providedFrames = framesPerBuffer * as_unsigned(this->inputChannels);
        auto begI = static_cast<float const*>(inputBuffer);
        this->audioQueue.emplace(begI, std::next(begI, as_signed(providedFrames)));
    }
    return paContinue;
}

void PortAudioProducer::stopRecording() {
    // Stop the recording
    if (this->inputStream) {
        try {
            if (this->inputStream->isActive()) {
                this->inputStream->stop();
            }
        } catch (const portaudio::PaException& e) {
            g_message("PortAudioProducer: Closing stream failed: %s", e.what());
        }
    }

    // Notify the consumer at the other side that there will be no more data
    this->audioQueue.signalEndOfStream();

    // Allow new recording by removing the old one
    this->inputStream.reset();
}
