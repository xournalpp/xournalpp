#include "VorbisConsumer.h"

#include <algorithm>
#include <cmath>

auto VorbisConsumer::start(const string& filename) -> bool {
    auto [sampleRate, channels] = this->audioQueue.getAudioAttributes();

    if (sampleRate == -1) {
        g_warning("VorbisConsumer: Timing issue - Sample rate requested before known");
        return false;
    }

    SF_INFO sfInfo;
    sfInfo.channels = channels;
    sfInfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
    sfInfo.samplerate = static_cast<int>(this->settings.getAudioSampleRate());

    auto SNDFILE_closer = [](SNDFILE* tag) { sf_close(tag); };
    std::unique_ptr<SNDFILE, decltype(SNDFILE_closer)> sfFile{sf_open(filename.c_str(), SFM_WRITE, &sfInfo),
                                                              std::move(SNDFILE_closer)};
    if (!sfFile) {
        g_warning("VorbisConsumer: output file \"%s\" could not be opened\ncaused by:%s", filename.c_str(),
                  sf_strerror(sfFile.get()));
        return false;
    }

    this->consumerThread = std::thread([this, sfFile = std::move(sfFile), channels = channels] {
        auto lock{audioQueue.acquire_lock()};
        auto buffer_size{static_cast<size_t>(std::max(0, 64 * channels))};
        std::vector<float> buffer;
        buffer.reserve(buffer_size);  // efficiency
        double audioGain = this->settings.getAudioGain();

        while (!(this->stopConsumer || (audioQueue.hasStreamEnded() && audioQueue.empty()))) {
            audioQueue.waitForProducer(lock);
            while (audioQueue.size() > buffer_size || (audioQueue.hasStreamEnded() && !audioQueue.empty())) {
                buffer.resize(0);
                this->audioQueue.pop(std::back_inserter(buffer), buffer_size);
                // apply gain
                if (audioGain != 1.0) {
                    std::for_each(begin(buffer), end(buffer), [audioGain](auto& val) { val *= audioGain; });
                }
                sf_writef_float(sfFile.get(), buffer.data(), std::min<size_t>(buffer.size() / channels, 64));
            }
        }
    });
    return true;
}

void VorbisConsumer::join() {
    // Join the consumer thread to wait for completion
    if (this->consumerThread.joinable()) {
        this->consumerThread.join();
    }
}

void VorbisConsumer::stop() {
    // Stop consumer
    this->audioQueue.signalEndOfStream();

    // Wait for consumer to finish
    join();
}
