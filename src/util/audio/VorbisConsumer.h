/*
 * Xournal++
 *
 * Class to save audio data in an opus file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <atomic>
#include <fstream>
#include <thread>
#include <utility>

#include <sndfile.h>

#include "control/settings/Settings.h"

#include "AudioQueue.h"
#include "DeviceInfo.h"

class VorbisConsumer final {
public:
    explicit VorbisConsumer(Settings& settings, AudioQueue<float>& audioQueue):
            settings(settings), audioQueue(audioQueue) {}

public:
    bool start(const string& filename);
    void join();
    void stop();

private:
    Settings& settings;
    AudioQueue<float>& audioQueue;

    std::thread consumerThread{};
    std::atomic<bool> stopConsumer{false};
};
