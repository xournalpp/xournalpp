/*
 * Xournal++
 *
 * Factory to initialize the audio system
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <functional>
#include <memory>

#include <portaudiocpp/PortAudioCpp.hxx>  // for AutoSystem

#include "audio/AudioPlayer.h"    // for AudioPlayer
#include "audio/AudioRecorder.h"  // for AudioRecorder

struct AudioSettings;

namespace xoj::audio {
class System {
public:
    System(const AudioSettings& settings, std::function<void()> onStopPlaying);
    virtual ~System();

private:
    /// RAII initializer don't move below the portaudio::System::instance() calls in AudioRecorder and AudioPlayer
    portaudio::AutoSystem autoSys;

public:
    AudioRecorder recorder;
    AudioPlayer player;
};
}  // namespace xoj::audio

extern "C" {
struct AudioFactory {
    std::unique_ptr<xoj::audio::System> (*makeAudio)(const AudioSettings& settings,
                                                     std::function<void()> onStopPlaying);
};
}
