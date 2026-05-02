/*
 * Xournal++
 *
 * Audio Recording / Playing controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "config-features.h"  // for ENABLE_AUDIO
#ifdef ENABLE_AUDIO

#include <cstddef>  // for size_t
#include <memory>   // for make_unique, unique_ptr
#include <vector>   // for vector

#include "LibraryHandle.h"
#include "filesystem.h"  // for path

class Control;
class DeviceInfo;
class Settings;
class AudioPlayer;
class AudioRecorder;

namespace xoj::audio {
class System;
}

class AudioController final {
    // Keep private: use the factory below
    AudioController(Settings* settings, Control* control, xoj::runtime::LibraryHandle audioLib,
                    std::unique_ptr<xoj::audio::System> audio);

public:
    /// Creates an AudioController instance if PortAudio (and deps) is found at runtime
    static std::unique_ptr<AudioController> tryLoadingAudioLibrary(Settings* settings, Control* control);
    ~AudioController();

    bool startRecording();
    bool stopRecording();
    bool isRecording();

    bool isPlaying();
    bool startPlayback(fs::path const& file, unsigned int timestamp);
    void pausePlayback();
    void continuePlayback();
    void stopPlayback();
    void seekForwards();
    void seekBackwards();

    fs::path const& getAudioFilename() const;
    fs::path getAudioFolder() const;
    size_t getStartTime() const;
    std::vector<DeviceInfo> getOutputDevices() const;
    std::vector<DeviceInfo> getInputDevices() const;

private:
    Settings& settings;
    Control& control;

    xoj::runtime::LibraryHandle audioLibrary;
    std::unique_ptr<xoj::audio::System> audio;
    AudioRecorder* audioRecorder;
    AudioPlayer* audioPlayer;

    fs::path audioFilename;
    size_t timestamp = 0;
};
#else
class AudioController final {};
#endif
