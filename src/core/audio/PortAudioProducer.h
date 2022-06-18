/*
 * Xournal++
 *
 * Class to record audio using libportaudio
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

class Settings;
template <typename T>
class AudioQueue;


class PortAudioProducer {
public:
    PortAudioProducer(Settings& settings, AudioQueue<float>& audioQueue): settings(settings), audioQueue(audioQueue){};

    std::vector<DeviceInfo> getInputDevices() const;

    DeviceInfo getSelectedInputDevice() const;

    bool isRecording() const;

    bool startRecording();

    int recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    void stopRecording();

private:
    portaudio::System& sys{portaudio::System::instance()};
    Settings& settings;

    AudioQueue<float>& audioQueue;

    std::unique_ptr<portaudio::MemFunCallbackStream<PortAudioProducer>> inputStream;

    int inputChannels = 0;
};
