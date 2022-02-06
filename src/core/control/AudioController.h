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

#include <string>
#include <vector>

#include "audio/AudioPlayer.h"
#include "audio/AudioRecorder.h"
#include "control/settings/Settings.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"

#include "Control.h"
#include "filesystem.h"

class AudioPlayer;

class AudioController final {
public:
    // Todo convert Pointers to reference (changes to control.cpp are necessary)
    AudioController(Settings* settings, Control* control): settings(*settings), control(*control) {}

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

    /**
     * RAII initializer don't move below the portaudio::System::instance() calls in
     * AudioRecorder and AudioPlayer
     * */
    portaudio::AutoSystem autoSys;
    std::unique_ptr<AudioRecorder> audioRecorder = std::make_unique<AudioRecorder>(settings);
    std::unique_ptr<AudioPlayer> audioPlayer = std::make_unique<AudioPlayer>(control, settings);

    fs::path audioFilename;
    size_t timestamp = 0;
};
