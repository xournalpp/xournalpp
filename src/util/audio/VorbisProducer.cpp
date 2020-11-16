#include "VorbisProducer.h"

constexpr auto sample_buffer_size = size_t{16384U};

auto VorbisProducer::start(const std::string& filename, unsigned int timestamp) -> bool {
    SF_INFO sfInfo{};
    auto SNDFILE_tag_deleter = [](SNDFILE_tag* p) { sf_close(p); };
    std::unique_ptr<SNDFILE_tag, decltype(SNDFILE_tag_deleter)> sfFile = {sf_open(filename.c_str(), SFM_READ, &sfInfo),
                                                                          SNDFILE_tag_deleter};
    if (!sfFile) {
        g_warning("VorbisProducer: input file \"%s\" could not be opened\ncaused by:%s", filename.c_str(),
                  sf_strerror(sfFile.get()));
        return false;
    }

    sf_count_t seekPosition = sfInfo.samplerate / 1000 * timestamp;

    if (seekPosition < sfInfo.frames) {
        sf_seek(sfFile.get(), seekPosition, SEEK_SET);
    } else {
        g_warning("VorbisProducer: Seeking outside of audio file extent");
    }

    this->audioQueue.setAudioAttributes(sfInfo.samplerate, static_cast<unsigned int>(sfInfo.channels));

    this->producerThread = std::thread([this, sfInfo, sfFile = std::move(sfFile)] {
        size_t numFrames{1};
        size_t const bufferSize{size_t(1024U) * sfInfo.channels};
        std::vector<float> sampleBuffer(bufferSize);
        auto lock = audioQueue.aquire_lock();

        while (!this->stopProducer && numFrames > 0 && !this->audioQueue.hasStreamEnded()) {
            sampleBuffer.resize(bufferSize);
            numFrames = sf_readf_float(sfFile.get(), sampleBuffer.data(), 1024);
            sampleBuffer.resize(numFrames * sfInfo.channels);

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
