/*
 * Xournal++
 *
 * Class to play audio using libportaudio
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <portaudiocpp/PortAudioCpp.hxx>  // for MemFunCallbackStream, System

#include "DeviceInfo.h"  // for DeviceInfo

class AudioPlayer;
template <typename T>
class AudioQueue;

class PortAudioConsumer final {
public:
    explicit PortAudioConsumer(AudioPlayer& audioPlayer, AudioQueue<float>& audioQueue):
            audioPlayer(audioPlayer), audioQueue(audioQueue) {}

    std::vector<DeviceInfo> getOutputDevices() const;
    DeviceInfo getSelectedOutputDevice() const;
    bool isPlaying() const;
    bool startPlaying();
    int playCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    void stopPlaying();

private:
    portaudio::System& sys{portaudio::System::instance()};
    AudioPlayer& audioPlayer;
    AudioQueue<float>& audioQueue;

    std::unique_ptr<portaudio::MemFunCallbackStream<PortAudioConsumer>> outputStream;

    int outputChannels = 0;
};
