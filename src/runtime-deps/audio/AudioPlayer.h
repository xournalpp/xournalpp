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

#include <functional>
#include <memory>  // for make_unique, unique_ptr
#include <vector>  // for vector

#include "filesystem.h"  // for path

template <typename T>
class AudioQueue;
class DeviceInfo;
class PortAudioConsumer;
struct AudioSettings;
class VorbisProducer;


class AudioPlayer {
public:
    explicit AudioPlayer(const AudioSettings& settings, std::function<void()> onStop);
    AudioPlayer(AudioPlayer const&) = delete;
    AudioPlayer(AudioPlayer&&) = delete;
    auto operator=(AudioPlayer const&) -> AudioPlayer& = delete;
    auto operator=(AudioPlayer&&) -> AudioPlayer& = delete;
    virtual ~AudioPlayer();

    virtual bool start(fs::path const& file, unsigned int timestamp = 0);
    [[nodiscard]] virtual bool isPlaying() const;
    virtual void stop();
    virtual bool play();
    virtual void pause();
    virtual void seek(int seconds);

    virtual std::vector<DeviceInfo> getOutputDevices();

    const AudioSettings& getSettings() const;
    void fireOnStop();

private:
    const AudioSettings& settings;

    std::unique_ptr<AudioQueue<float>> audioQueue;
    std::unique_ptr<PortAudioConsumer> portAudioConsumer;
    std::unique_ptr<VorbisProducer> vorbisProducer;

    std::function<void()> onStop;
};
