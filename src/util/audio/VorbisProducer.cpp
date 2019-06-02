#include "VorbisProducer.h"

VorbisProducer::VorbisProducer(AudioQueue<float>* audioQueue) : audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(VorbisProducer);
}

VorbisProducer::~VorbisProducer()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	XOJ_RELEASE_TYPE(VorbisProducer);
}

bool VorbisProducer::start(std::string filename, unsigned int timestamp)
{
	XOJ_CHECK_TYPE(VorbisProducer);

	this->sfInfo.format = 0;
	this->sfFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
	if (sfFile == nullptr)
	{
		g_warning("VorbisProducer: input file \"%s\" could not be opened\ncaused by:%s", filename.c_str(), sf_strerror(sfFile));
		return false;
	}

	sf_count_t seekPosition = this->sfInfo.samplerate / 1000 * timestamp;
	if (seekPosition < this->sfInfo.frames)
	{
		sf_seek(this->sfFile, seekPosition, SEEK_SET);
	}
	else
	{
		g_warning("VorbisProducer: Seeking outside of audio file extent");
	}

	this->audioQueue->setAudioAttributes(this->sfInfo.samplerate, static_cast<unsigned int>(this->sfInfo.channels));

	this->producerThread = new std::thread(
			[&, filename]
			{
				long numSamples = 1;
				auto sampleBuffer = new float[1024 * this->sfInfo.channels];

				while (!this->stopProducer && numSamples > 0 && !this->audioQueue->hasStreamEnded())
				{
					numSamples = sf_readf_float(this->sfFile, sampleBuffer, 1024);

					while (this->audioQueue->size() >= 16384 && !this->audioQueue->hasStreamEnded() && !this->stopProducer)
					{
						std::this_thread::sleep_for(std::chrono::microseconds(100));
					}

					this->audioQueue->push(sampleBuffer, static_cast<unsigned long>(numSamples * this->sfInfo.channels));
				}
				this->audioQueue->signalEndOfStream();

				delete[] sampleBuffer;
				sampleBuffer = nullptr;

				sf_close(this->sfFile);
			});
	return true;
}

void VorbisProducer::abort()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	this->stopProducer = true;
	// Wait for producer to finish
	stop();
	this->stopProducer = false;

}

void VorbisProducer::stop()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	// Wait for producer to finish
	if (this->producerThread && this->producerThread->joinable())
	{
		this->producerThread->join();
	}
}
