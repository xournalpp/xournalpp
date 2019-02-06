#include "VorbisProducer.h"

VorbisProducer::VorbisProducer(AudioQueue<int>* audioQueue) : audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(VorbisProducer);
}

VorbisProducer::~VorbisProducer()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	XOJ_RELEASE_TYPE(VorbisProducer);
}

void VorbisProducer::start(std::string filename, const DeviceInfo& outputDevice, unsigned int timestamp)
{
	XOJ_CHECK_TYPE(VorbisProducer);

	this->sfInfo.format = 0;
	this->sfFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
	if (sfFile == nullptr)
	{
		g_warning("VorbisProducer: input file \"%s\" could not be opened\ncaused by:%s", filename.c_str(), sf_strerror(sfFile));
		return;
	}

	//TODO implement seeking (this is hard since we need to get the frame offset)

	this->producerThread = new std::thread(
			[&, filename, timestamp]
			{
				long numSamples = 1;
				auto sampleBuffer = new int[1024 * this->sfInfo.channels];

				while (!this->stopProducer && numSamples > 0 && !this->audioQueue->hasStreamEnded())
				{
					numSamples = sf_readf_int(this->sfFile, sampleBuffer, 1024);

					while (this->audioQueue->size() > 4096 && !this->audioQueue->hasStreamEnded())
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
}

SF_INFO* VorbisProducer::getSignalInformation()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	return &this->sfInfo;
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