/*
 * Xournal++
 *
 * Class to record audio and store it as MP3-file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "control/settings/Settings.h"

#include "AudioQueue.h"
#include "PortAudioProducer.h"
#include "VorbisConsumer.h"
#include "filesystem.h"

struct AudioRecorder final {
    explicit AudioRecorder(Settings& settings): settings(settings) {}
    AudioRecorder(AudioRecorder const&) = delete;
    AudioRecorder(AudioRecorder&&) = delete;
    auto operator=(AudioRecorder const&) -> AudioRecorder& = delete;
    auto operator=(AudioRecorder&&) -> AudioRecorder& = delete;
    ~AudioRecorder();

    bool start(fs::path const& file);
    void stop();
    bool isRecording() const;
    std::vector<DeviceInfo> getInputDevices() const;

private:
    Settings& settings;

    std::unique_ptr<AudioQueue<float>> audioQueue = std::make_unique<AudioQueue<float>>();
    std::unique_ptr<PortAudioProducer> portAudioProducer = std::make_unique<PortAudioProducer>(settings, *audioQueue);
    std::unique_ptr<VorbisConsumer> vorbisConsumer = std::make_unique<VorbisConsumer>(settings, *audioQueue);
};
