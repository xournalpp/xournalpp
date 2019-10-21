#include "VorbisProducer.h"


VorbisProducer::VorbisProducer(AudioQueue<float>* audioQueue) : audioQueue(audioQueue)
{
}

VorbisProducer::~VorbisProducer()
{
}

bool VorbisProducer::start(std::string filename, unsigned int timestamp)
{
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
				size_t numFrames = 1;
				auto sampleBuffer = new float[1024 * this->sfInfo.channels];
				std::unique_lock<std::mutex> lock(audioQueue->syncMutex());
								
				while (!this->stopProducer && numFrames > 0 && !this->audioQueue->hasStreamEnded())
				{
					numFrames = sf_readf_float(this->sfFile, sampleBuffer, 1024);

					while (this->audioQueue->size() >= this->sample_buffer_size && !this->audioQueue->hasStreamEnded() && !this->stopProducer)
					{
						audioQueue->waitForConsumer(lock);
					}

					if (this->seekSeconds != 0)
					{
						sf_seek(this->sfFile,seekSeconds * this->sfInfo.samplerate, SEEK_CUR);
						this->seekSeconds = 0;
					}

					this->audioQueue->push(sampleBuffer, static_cast<size_t>(numFrames * this->sfInfo.channels));
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
	this->stopProducer = true;
	// Wait for producer to finish
	stop();
	this->stopProducer = false;

}

void VorbisProducer::stop()
{
	// Wait for producer to finish
	if (this->producerThread && this->producerThread->joinable())
	{
		this->producerThread->join();
	}
}


void VorbisProducer::seek(int seconds)
{
	this->seekSeconds = seconds;
}