#include "PortAudioConsumer.h"

PortAudioConsumer::PortAudioConsumer(Settings* settings, AudioQueue<int>* audioQueue) : sys(portaudio::System::instance()), settings(settings), audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(PortAudioConsumer);
}

PortAudioConsumer::~PortAudioConsumer()
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	if (portaudio::System::exists())
	{
		portaudio::System::terminate();
	}

	XOJ_RELEASE_TYPE(PortAudioConsumer);
}

std::list<DeviceInfo> PortAudioConsumer::getOutputDevices()
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	std::list<DeviceInfo> deviceList;

	for (portaudio::System::DeviceIterator i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i)
	{

		if (i->isFullDuplexDevice() || i->isOutputOnlyDevice())
		{
			DeviceInfo deviceInfo(&(*i), this->settings->getAudioOutputDevice() == i->index());
			deviceList.push_back(deviceInfo);
		}

	}
	return deviceList;
}

const DeviceInfo PortAudioConsumer::getSelectedOutputDevice()
{
	try
	{
		return DeviceInfo(&sys.deviceByIndex(this->settings->getAudioOutputDevice()), true);
	}
	catch (portaudio::PaException& e)
	{
		g_warning("PortAudioConsumer: Selected output device was not found - fallback to default output device\nCaused by: %s", e.what());
		return DeviceInfo(&sys.defaultOutputDevice(), true);
	}
}

bool PortAudioConsumer::isPlaying()
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	return this->outputStream != nullptr && this->outputStream->isActive();
}

void PortAudioConsumer::startPlaying(double sampleRate, unsigned int channels)
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	// Check if there already is a recording
	if (this->outputStream != nullptr)
	{
		return;
	}

	// Get the device information of our output device
	portaudio::Device* device = nullptr;
	try
	{
		device = &sys.deviceByIndex(getSelectedOutputDevice().getIndex());
	}
	catch (portaudio::PaException& e)
	{
		g_warning("PortAudioConsumer: Unable to find selected output device");
		return;
	}

	if (device->maxOutputChannels() < channels)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("Output device has not enough channels to play audio file. (Requires at least 2 channels)");
		return;
	}

	this->outputChannels = channels;
	portaudio::DirectionSpecificStreamParameters outParams(*device, channels, portaudio::INT32, true, device->defaultLowOutputLatency(), nullptr);
	portaudio::StreamParameters params(portaudio::DirectionSpecificStreamParameters::null(), outParams, sampleRate, this->framesPerBuffer, paNoFlag);

	try
	{
		this->outputStream = new portaudio::MemFunCallbackStream<PortAudioConsumer>(params, *this, &PortAudioConsumer::playCallback);
	}
	catch (portaudio::PaException& e)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("PortAudioConsumer: Unable to open stream to device");
		return;
	}
	// Start the recording
	try
	{
		this->outputStream->start();
	}
	catch (portaudio::PaException& e)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("PortAudioConsumer: Unable to start stream");
		return;
	}
}

int PortAudioConsumer::playCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
									PaStreamCallbackFlags statusFlags)
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	if (statusFlags)
	{
		g_warning("PortAudioConsumer: statusFlag: %s", std::to_string(statusFlags).c_str());
	}

	if (outputBuffer != nullptr)
	{
		int outputBufferLength;
		this->audioQueue->pop(((int*) outputBuffer), &outputBufferLength, framesPerBuffer * this->outputChannels, this->outputChannels);

		// Fill buffer to requested length if necessary

		if (outputBufferLength < framesPerBuffer * this->outputChannels)
		{
			g_warning("PortAudioConsumer: Frame underflow");

			auto outputBufferImpl = (int*) outputBuffer;
			for (int i = outputBufferLength; i < framesPerBuffer * this->outputChannels; ++i)
			{
				outputBufferImpl[i] = 0;
			}
		}


		if (!this->audioQueue->hasStreamEnded() || !this->audioQueue->empty())
		{
			return paContinue;
		}
	}
	return paComplete;
}

void PortAudioConsumer::stopPlaying()
{
	XOJ_CHECK_TYPE(PortAudioConsumer);

	// Stop the playback
	if (this->outputStream != nullptr)
	{
		try
		{
			if (this->outputStream->isActive())
			{
				this->outputStream->stop();
			}
			if (this->outputStream->isOpen())
			{
				this->outputStream->close();
			}
		}
		catch (portaudio::PaException& e)
		{
			g_warning("PortAudioConsumer: Closing stream failed");
		}
	}

	// Allow new playback by removing the old one
	delete this->outputStream;
	this->outputStream = nullptr;
}
