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

#include <atomic>  // for atomic
#include <thread>  // for thread

#include "filesystem.h"  // for path

struct AudioSettings;
template <typename T>
class AudioQueue;

class VorbisConsumer final {
public:
    explicit VorbisConsumer(const AudioSettings& settings, AudioQueue<float>& audioQueue):
            settings(settings), audioQueue(audioQueue) {}

public:
    bool start(fs::path const& file);
    void join();
    void stop();

private:
    const AudioSettings& settings;
    AudioQueue<float>& audioQueue;


    std::thread consumerThread{};
    std::atomic<bool> stopConsumer{false};
};
