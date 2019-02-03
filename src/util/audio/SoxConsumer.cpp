#include "SoxConsumer.h"

SoxConsumer::SoxConsumer(Settings *settings, AudioQueue* audioQueue)
 : settings(settings),
   audioQueue(audioQueue)
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

void SoxConsumer::start(string filename, unsigned int inputChannels)
{
    XOJ_CHECK_TYPE(SoxConsumer);

    this->inputSignal = new sox_signalinfo_t();
    this->inputSignal->rate = this->settings->getAudioSampleRate();
    this->inputSignal->length = SOX_UNSPEC;
    this->inputSignal->channels = inputChannels;
    this->inputSignal->mult = nullptr;
    this->inputSignal->precision = 32;

    this->outputFile = sox_open_write(filename.c_str(), this->inputSignal, nullptr, nullptr, nullptr, nullptr);

    if (this->outputFile == nullptr)
    {
        // TODO Stop recording in this case, so the users sees that recording is not running!
        g_warning("SoxConsumer: output file \"%s\" could not be opened", filename.c_str());
        return;
    }

    int buffer[64 * this->inputSignal->channels];
    int bufferLength;

    this->consumerThread = new std::thread([&]
       {
           std::unique_lock<std::mutex> lock(audioQueue->syncMutex());
           while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
           {
               audioQueue->waitForNewElements(lock);

               while (!audioQueue->empty())
               {
                   audioQueue->pop(buffer, &bufferLength, 64ul * this->inputSignal->channels, this->inputSignal->channels);

                   if (bufferLength > 0)
                   {
                       sox_write(this->outputFile, buffer, (unsigned int) bufferLength);
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
