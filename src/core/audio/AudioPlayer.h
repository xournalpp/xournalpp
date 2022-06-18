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

#include <memory>  // for make_unique, unique_ptr
#include <vector>  // for vector

#include "filesystem.h"  // for path

template <typename T>
class AudioQueue;
class Control;
class DeviceInfo;
class PortAudioConsumer;
class Settings;
class VorbisProducer;


class AudioPlayer final {
public:
    explicit AudioPlayer(Control& control, Settings& settings);
    AudioPlayer(AudioPlayer const&) = delete;
    AudioPlayer(AudioPlayer&&) = delete;
    auto operator=(AudioPlayer const&) -> AudioPlayer& = delete;
    auto operator=(AudioPlayer&&) -> AudioPlayer& = delete;
    ~AudioPlayer();

    bool start(fs::path const& file, unsigned int timestamp = 0);
    bool isPlaying();
    void stop();
    bool play();
    void pause();
    void seek(int seconds);

    std::vector<DeviceInfo> getOutputDevices();

    Settings& getSettings();
    void disableAudioPlaybackButtons();

private:
    Control& control;
    Settings& settings;

    std::unique_ptr<AudioQueue<float>> audioQueue;
    std::unique_ptr<PortAudioConsumer> portAudioConsumer;
    std::unique_ptr<VorbisProducer> vorbisProducer;
};
