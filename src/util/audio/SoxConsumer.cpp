#include "SoxConsumer.h"

SoxConsumer::SoxConsumer(AudioQueue* audioQueue)
 : audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(SoxConsumer);

	sox_init();
	sox_format_init();
}

SoxConsumer::~SoxConsumer()
{
	XOJ_CHECK_TYPE(SoxConsumer);

	sox_format_quit();
	sox_quit();

	XOJ_RELEASE_TYPE(SoxConsumer);
}

void SoxConsumer::start(string filename, double sampleRate, const DeviceInfo& inputDevice)
{
	XOJ_CHECK_TYPE(SoxConsumer);

	this->inputSignal = new sox_signalinfo_t();
	this->inputSignal->rate = sampleRate;
	this->inputSignal->length = SOX_UNSPEC;
	this->inputSignal->channels = (unsigned int) inputDevice.getInputChannels();
	this->inputSignal->mult = nullptr;
	this->inputSignal->precision = 32;

	this->outputFile = sox_open_write(filename.c_str(), this->inputSignal, nullptr, nullptr, nullptr, nullptr);

	if (this->outputFile == nullptr)
	{
		// TODO Stop recording in this case, so the users sees that recording is not running!
		g_warning("SoxConsumer: output file \"%s\" could not be opened", filename.c_str());
		return;
	}

	this->consumerThread = new std::thread([&]
		{
			std::unique_lock<std::mutex> lock(audioQueue->syncMutex());
			while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
			{
				audioQueue->waitForNewElements(lock);

				while (!audioQueue->empty())
				{
					std::vector<int> tmpBuffer = audioQueue->pop(64ul * this->inputSignal->channels);

					if (!tmpBuffer.empty())
					{
						sox_write(this->outputFile, tmpBuffer.data(), tmpBuffer.size());
					}
				}
			}

			sox_close(this->outputFile);
		});
}

void SoxConsumer::join()
{
	XOJ_CHECK_TYPE(SoxConsumer);

	// Join the consumer thread to wait for completion
	if (this->consumerThread && this->consumerThread->joinable())
	{
		this->consumerThread->join();
	}
}

void SoxConsumer::stop()
{
	XOJ_CHECK_TYPE(SoxConsumer);

	// Stop consumer
	this->audioQueue->signalEndOfStream();

	// Wait for consumer to finish
	join();
}
