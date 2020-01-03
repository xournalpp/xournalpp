/*
 * Xournal++
 *
 * Class to play audio from a MP3-file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "control/Control.h"
#include "control/settings/Settings.h"

#include "AudioQueue.h"
#include "PortAudioConsumer.h"
#include "VorbisProducer.h"
#include "XournalType.h"

class AudioPlayer final {
public:
    explicit AudioPlayer(Control& control, Settings& settings): control(control), settings(settings) {}

    ~AudioPlayer();
    bool start(const string& filename, unsigned int timestamp = 0);
    bool isPlaying();
    void stop();
    bool play();
    void pause();
    void seek(int seconds);

    vector<DeviceInfo> getOutputDevices();

    Settings& getSettings();
    void disableAudioPlaybackButtons();

private:
    Control& control;
    Settings& settings;

    std::unique_ptr<AudioQueue<float>> audioQueue = std::make_unique<AudioQueue<float>>();
    std::unique_ptr<PortAudioConsumer> portAudioConsumer = std::make_unique<PortAudioConsumer>(*this, *audioQueue);
    std::unique_ptr<VorbisProducer> vorbisProducer = std::make_unique<VorbisProducer>(*audioQueue);
};
