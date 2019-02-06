#include "VorbisConsumer.h"

VorbisConsumer::VorbisConsumer(Settings* settings, AudioQueue<int>* audioQueue)
		: settings(settings),
		  audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(VorbisConsumer);
}

VorbisConsumer::~VorbisConsumer()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	XOJ_RELEASE_TYPE(VorbisConsumer);
}

bool VorbisConsumer::start(string filename, unsigned int inputChannels)
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	this->consumerThread = new std::thread(
			[&, filename, inputChannels]
			{
				std::unique_lock<std::mutex> lock(audioQueue->syncMutex());

				SF_INFO sfInfo;
				sfInfo.channels = inputChannels;
				sfInfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
				sfInfo.samplerate = static_cast<int>(this->settings->getAudioSampleRate());

				SNDFILE_tag* sfFile = sf_open(filename.c_str(), SFM_WRITE, &sfInfo);
				if (sfFile == nullptr)
				{
					g_warning("VorbisConsumer: output file \"%s\" could not be opened\ncaused by:%s", filename.c_str(), sf_strerror(sfFile));
					return;
				}

				int buffer[64 * inputChannels];
				int bufferLength;

				while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
				{
					audioQueue->waitForNewElements(lock);

					while (!audioQueue->empty())
					{
						this->audioQueue->pop(buffer, &bufferLength, 64 * inputChannels, inputChannels);
						sf_writef_int(sfFile, buffer, 64);
					}
				}

				sf_close(sfFile);
			});
	return true;
}

void VorbisConsumer::join()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	// Join the consumer thread to wait for completion
	if (this->consumerThread && this->consumerThread->joinable())
	{
		this->consumerThread->join();
	}
}

void VorbisConsumer::stop()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	// Stop consumer
	this->audioQueue->signalEndOfStream();

	// Wait for consumer to finish
	join();
}
