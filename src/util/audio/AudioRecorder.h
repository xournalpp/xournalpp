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
#include "XournalType.h"

struct AudioRecorder {
    explicit AudioRecorder(Settings& settings): settings(settings) {}
    ~AudioRecorder();

    bool start(const string& filename);
    void stop();
    bool isRecording() const;
    vector<DeviceInfo> getInputDevices() const;

private:
    Settings& settings;

    std::unique_ptr<AudioQueue<float>> audioQueue = std::make_unique<AudioQueue<float>>();
    std::unique_ptr<PortAudioProducer> portAudioProducer = std::make_unique<PortAudioProducer>(settings, *audioQueue);
    std::unique_ptr<VorbisConsumer> vorbisConsumer = std::make_unique<VorbisConsumer>(settings, *audioQueue);
};
