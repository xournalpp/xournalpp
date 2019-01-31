#include "SoxConsumer.h"

SoxConsumer::SoxConsumer(AudioQueue *audioQueue) : audioQueue(audioQueue)
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

void SoxConsumer::start(std::string filename, double sampleRate, const DeviceInfo &inputDevice)
{
    XOJ_CHECK_TYPE(SoxConsumer);

    this->inputSignal = new sox_signalinfo_t;
    this->inputSignal->rate = sampleRate;
    this->inputSignal->length = SOX_UNSPEC;
    this->inputSignal->channels = (unsigned int) inputDevice.inputChannels;
    this->inputSignal->mult = nullptr;
    this->inputSignal->precision = 32;

    this->outputFile = sox_open_write(filename.c_str(), this->inputSignal, nullptr, nullptr, nullptr, nullptr);

    if (this->outputFile == nullptr)
    {
        g_message("SoxConsumer: output file could not be opened");
        return;
    }

    this->consumerThread = new std::thread([&]
       {
           unsigned long availableFrames;

           while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
           {
               audioQueue->waitForNewElements();

               while (!audioQueue->empty())
               {
                   unsigned long queueSize = audioQueue->size();
                   availableFrames = std::min(queueSize - queueSize % this->inputSignal->channels, (unsigned long) (64 * this->inputSignal->channels));
                   if (availableFrames > 0)
                   {
                       std::vector<int> tmpBuffer  = audioQueue->pop(availableFrames);
                       sox_write(this->outputFile, tmpBuffer.data(), availableFrames);
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
    if (this->consumerThread->joinable())
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
    if (this->consumerThread->joinable())
    {
        this->consumerThread->join();
    }

}
