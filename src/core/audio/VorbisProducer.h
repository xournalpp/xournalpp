/*
 * Xournal++
 *
 * Class to read audio data from an mp3 file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <atomic>
#include <thread>
#include <utility>

#include "AudioQueue.h"
#include "DeviceInfo.h"
#include "filesystem.h"

struct VorbisProducer final {
    explicit VorbisProducer(AudioQueue<float>& audioQueue): audioQueue(audioQueue) {}

    bool start(fs::path const& file, unsigned int timestamp);
    void abort();
    void stop();
    void seek(int seconds);

private:
    AudioQueue<float>& audioQueue;
    std::thread producerThread{};

    std::atomic<bool> stopProducer{false};
    std::atomic<int> seekSeconds{0};
};
