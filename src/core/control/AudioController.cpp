#include "AudioController.h"

#include <array>   // for array
#include <cstdio>  // for snprintf
#include <ctime>   // for tm, localtime, time
#include <string>  // for string, allocator

#include <glib.h>  // for g_get_monotonic_time

#include "audio/AudioPlayer.h"                   // for AudioPlayer
#include "audio/AudioRecorder.h"                 // for AudioRecorder
#include "audio/DeviceInfo.h"                    // for DeviceInfo
#include "control/Control.h"                     // for Control
#include "control/settings/Settings.h"           // for Settings
#include "gui/MainWindow.h"                      // for MainWindow
#include "gui/toolbarMenubar/ToolMenuHandler.h"  // for ToolMenuHandler
#include "util/XojMsgBox.h"                      // for XojMsgBox
#include "util/i18n.h"                           // for _


using std::string;
using std::vector;

AudioController::AudioController(Settings* settings, Control* control):
        settings(*settings),
        control(*control),
        audioRecorder(std::make_unique<AudioRecorder>(*settings)),
        audioPlayer(std::make_unique<AudioPlayer>(*control, *settings)) {}

AudioController::~AudioController() = default;


auto AudioController::startRecording() -> bool {
    if (!this->isRecording()) {
        if (getAudioFolder().empty()) {
            return false;
        }

        this->timestamp = static_cast<size_t>(g_get_monotonic_time() / 1000);

        std::array<char, 50> buffer{};
        time_t secs = time(nullptr);
        tm* t = localtime(&secs);
        // This prints the date and time in ISO format.
        snprintf(buffer.data(), buffer.size(), "%04d-%02d-%02d_%02d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1,
                 t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        string data(buffer.data());
        data += ".ogg";

        audioFilename = data;

        g_message("Start recording");

        bool isRecording = this->audioRecorder->start(getAudioFolder() / data);

        if (!isRecording) {
            audioFilename = "";
            this->timestamp = 0;
        }

        return isRecording;
    }
    return false;
}

auto AudioController::stopRecording() -> bool {
    if (this->audioRecorder->isRecording()) {
        audioFilename = "";
        this->timestamp = 0;

        g_message("Stop recording");

        this->audioRecorder->stop();
    }
    return true;
}

auto AudioController::isRecording() -> bool { return this->audioRecorder->isRecording(); }

auto AudioController::isPlaying() -> bool { return this->audioPlayer->isPlaying(); }

auto AudioController::startPlayback(fs::path const& file, unsigned int timestamp) -> bool {
    this->audioPlayer->stop();
    bool status = this->audioPlayer->start(file, timestamp);
    if (status) {
        this->control.getWindow()->getToolMenuHandler()->enableAudioPlaybackButtons();
    }
    return status;
}

void AudioController::pausePlayback() {
    this->control.getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(true);

    this->audioPlayer->pause();
}

void AudioController::seekForwards() { this->audioPlayer->seek(this->settings.getDefaultSeekTime()); }

void AudioController::seekBackwards() { this->audioPlayer->seek(-1 * this->settings.getDefaultSeekTime()); }

void AudioController::continuePlayback() {
    this->control.getWindow()->getToolMenuHandler()->setAudioPlaybackPaused(false);

    this->audioPlayer->play();
}

void AudioController::stopPlayback() {
    this->control.getWindow()->getToolMenuHandler()->disableAudioPlaybackButtons();
    this->audioPlayer->stop();
}

auto AudioController::getAudioFilename() const -> fs::path const& { return this->audioFilename; }

auto AudioController::getAudioFolder() const -> fs::path {
    auto const& af = this->settings.getAudioFolder();

    if (!fs::is_directory(af)) {
        string msg = _("Audio folder not set or invalid! Recording won't work!\nPlease set the "
                       "recording folder under \"Preferences > Audio recording\"");
        g_warning("%s", msg.c_str());
        XojMsgBox::showErrorToUser(this->control.getGtkWindow(), msg);
        return fs::path{};
    }
    return af;
}

auto AudioController::getStartTime() const -> size_t { return this->timestamp; }

auto AudioController::getOutputDevices() const -> vector<DeviceInfo> { return this->audioPlayer->getOutputDevices(); }

auto AudioController::getInputDevices() const -> vector<DeviceInfo> { return this->audioRecorder->getInputDevices(); }
