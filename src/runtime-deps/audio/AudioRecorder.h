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

#include <memory>  // for make_unique, unique_ptr
#include <vector>  // for vector

#include "filesystem.h"  // for path

template <typename T>
class AudioQueue;
class DeviceInfo;
class PortAudioProducer;
struct AudioSettings;
class VorbisConsumer;

class AudioRecorder {
public:
    explicit AudioRecorder(const AudioSettings& settings);
    AudioRecorder(AudioRecorder const&) = delete;
    AudioRecorder(AudioRecorder&&) = delete;
    auto operator=(AudioRecorder const&) -> AudioRecorder& = delete;
    auto operator=(AudioRecorder&&) -> AudioRecorder& = delete;
    virtual ~AudioRecorder();

    virtual bool start(fs::path const& file);
    virtual void stop();
    [[nodiscard]] virtual bool isRecording() const;
    [[nodiscard]] virtual std::vector<DeviceInfo> getInputDevices() const;

private:
    std::unique_ptr<AudioQueue<float>> audioQueue;
    std::unique_ptr<PortAudioProducer> portAudioProducer;
    std::unique_ptr<VorbisConsumer> vorbisConsumer;
};
