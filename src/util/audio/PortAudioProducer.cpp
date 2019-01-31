#include "PortAudioProducer.h"

PortAudioProducer::PortAudioProducer(Settings *settings, AudioQueue *audioQueue) : sys(portaudio::System::instance()), settings(settings), audioQueue(audioQueue)
{
    XOJ_INIT_TYPE(PortAudioProducer);

    DeviceInfo inputInfo(&sys.defaultInputDevice(), true);
    this->setInputDevice(inputInfo);
}

PortAudioProducer::~PortAudioProducer()
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    portaudio::System::terminate();

    XOJ_RELEASE_TYPE(PortAudioProducer);
}

std::list<DeviceInfo> PortAudioProducer::getInputDevices()
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    std::list<DeviceInfo> deviceList;

    for (portaudio::System::DeviceIterator i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i)
    {

        if (i->isFullDuplexDevice() || i->isInputOnlyDevice())
        {
            DeviceInfo deviceInfo(&(*i), this->selectedInputDevice == i->index());
            deviceList.push_back(deviceInfo);
        }

    }
    return deviceList;
}

const DeviceInfo PortAudioProducer::getSelectedInputDevice()
{
    return DeviceInfo(&sys.deviceByIndex(this->selectedInputDevice), true);
}


void PortAudioProducer::setInputDevice(DeviceInfo deviceInfo)
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    this->selectedInputDevice = deviceInfo.getIndex();
    portaudio::Device *device = &sys.deviceByIndex(this->selectedInputDevice);
    this->inputChannels = static_cast<unsigned int>(device->maxInputChannels());
}

bool PortAudioProducer::isRecording()
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    return this->inputStream != nullptr && this->inputStream->isActive();
}

void PortAudioProducer::startRecording()
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    // Check if there already is a recording
    if (this->inputStream != nullptr)
    {
        return;
    }

    // Get the device information of our input device
    portaudio::Device *device = &sys.deviceByIndex(this->selectedInputDevice);
    portaudio::DirectionSpecificStreamParameters inParams(*device, device->maxInputChannels(), portaudio::INT32, true, device->defaultLowInputLatency(), nullptr);
    portaudio::StreamParameters params(inParams, portaudio::DirectionSpecificStreamParameters::null(), this->sampleRate, this->framesPerBuffer, paNoFlag);

    // Specify the callback used for buffering the recorded data
    this->inputStream = new portaudio::MemFunCallbackStream<PortAudioProducer>(params, *this, &PortAudioProducer::recordCallback);

    // Start the recording
    this->inputStream->start();
}

int PortAudioProducer::recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo,
                                      PaStreamCallbackFlags statusFlags)
{
    XOJ_CHECK_TYPE(PortAudioProducer);
    std::unique_lock<std::mutex> lock(this->audioQueue->syncMutex());

    if (statusFlags)
    {
        g_message("PortAudioProducer: statusFlag: %s", std::to_string(statusFlags).c_str());
    }

    if (inputBuffer != nullptr)
    {
        unsigned long providedFrames = framesPerBuffer * this->inputChannels;

        this->audioQueue->push(((int *) inputBuffer), providedFrames);
    }
    return paContinue;
}

void PortAudioProducer::stopRecording()
{
    XOJ_CHECK_TYPE(PortAudioProducer);

    // Stop the recording
    if (this->inputStream != nullptr)
    {
        this->inputStream->stop();
        this->inputStream->close();
    }

    // Notify the consumer at the other side that ther will be no more data
    this->audioQueue->signalEndOfStream();

    // Allow new recording by removing the old one
    delete this->inputStream;
    this->inputStream = nullptr;
}