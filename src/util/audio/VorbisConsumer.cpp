#include <cmath>
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

bool VorbisConsumer::start(string filename)
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	double sampleRate;
	unsigned int channels;
	this->audioQueue->getAudioAttributes(sampleRate, channels);

	if (sampleRate == -1)
	{
		g_warning("VorbisConsumer: Timing issue - Sample rate requested before known");
		return false;
	}

	SF_INFO sfInfo;
	sfInfo.channels = channels;
	sfInfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	sfInfo.samplerate = static_cast<int>(this->settings->getAudioSampleRate());

	SNDFILE_tag* sfFile = sf_open(filename.c_str(), SFM_WRITE, &sfInfo);
	if (sfFile == nullptr)
	{
		g_warning("VorbisConsumer: output file \"%s\" could not be opened\ncaused by:%s", filename.c_str(), sf_strerror(sfFile));
		return false;
	}

	this->consumerThread = new std::thread(
			[&, sfFile, channels]
			{
				std::unique_lock<std::mutex> lock(audioQueue->syncMutex());

				int buffer[64 * channels];
				unsigned long bufferLength;
				double audioGain = this->settings->getAudioGain();

				while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
				{
					audioQueue->waitForProducer(lock);

					while (!audioQueue->empty())
					{
						this->audioQueue->pop(buffer, bufferLength, 64 * channels);

						// apply gain
						if (audioGain != 1.0)
						{
							for (unsigned int i = 0; i < 64 * channels; ++i)
							{
								// check for overflow
								if (std::abs(buffer[i]) < std::floor(INT_MAX / audioGain))
								{
									buffer[i] = static_cast<int>(buffer[i] * audioGain);
								} else
								{
									// clip audio
									if (buffer[i] > 0)
									{
										buffer[i] = INT_MAX;
									} else
									{
										buffer[i] = INT_MIN;
									}
								}
							}
						}

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
