#include "VorbisProducer.h"

#include <glib.h>

#include "audio/SNDFileCpp.h"

using namespace xoj;

constexpr auto sample_buffer_size = size_t{16384U};

auto VorbisProducer::start(fs::path const& file, unsigned int timestamp) -> bool {
    SF_INFO sfInfo{};
    auto sfFile = audio::make_snd_file(file, SFM_READ, &sfInfo);
    if (!sfFile) {
        g_warning("VorbisProducer: input file \"%s\" could not be opened\ncaused by:%s", file.u8string().c_str(),
                  sf_strerror(sfFile.get()));
        return false;
    }

    sf_count_t seekPosition = sfInfo.samplerate / 1000 * sf_count_t(timestamp);

    if (seekPosition < sfInfo.frames) {
        sf_seek(sfFile.get(), seekPosition, SEEK_SET);
    } else {
        g_warning("VorbisProducer: Seeking outside of audio file extent");
    }

    this->audioQueue.setAudioAttributes(sfInfo.samplerate, static_cast<unsigned int>(sfInfo.channels));

    this->producerThread = std::thread([this, sfInfo, sfFile = std::move(sfFile)] {
        sf_count_t numFrames{1};
        size_t const bufferSize{size_t(1024U) * size_t(sfInfo.channels)};
        std::vector<float> sampleBuffer(bufferSize);
        auto lock = audioQueue.acquire_lock();

        while (!this->stopProducer && numFrames > 0 && !this->audioQueue.hasStreamEnded()) {
            sampleBuffer.resize(bufferSize);
            numFrames = sf_readf_float(sfFile.get(), sampleBuffer.data(), 1024);
            sampleBuffer.resize(size_t(numFrames * sfInfo.channels));

            while (this->audioQueue.size() >= sample_buffer_size && !this->audioQueue.hasStreamEnded() &&
                   !this->stopProducer) {
                audioQueue.waitForConsumer(lock);
            }

            if (auto tmpSeekSeconds = this->seekSeconds.load(); tmpSeekSeconds != 0) {
                sf_seek(sfFile.get(), tmpSeekSeconds * sfInfo.samplerate, SEEK_CUR);
                this->seekSeconds -= tmpSeekSeconds;
            }

            this->audioQueue.emplace(begin(sampleBuffer), end(sampleBuffer));
        }
        this->audioQueue.signalEndOfStream();
    });
    return true;
}

void VorbisProducer::abort() {
    this->stopProducer = true;
    // Wait for producer to finish
    stop();
    this->stopProducer = false;
}

void VorbisProducer::stop() {
    // Wait for producer to finish
    if (this->producerThread.joinable()) {
        this->producerThread.join();
    }
}


void VorbisProducer::seek(int seconds) { this->seekSeconds = seconds; }
