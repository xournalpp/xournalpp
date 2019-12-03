#include "PortAudioProducer.h"

PortAudioProducer::PortAudioProducer(Settings* settings, AudioQueue<float>* audioQueue)
		: sys(portaudio::System::instance()),
		  settings(settings),
		  audioQueue(audioQueue)
{
}

PortAudioProducer::~PortAudioProducer()
{
	if (portaudio::System::exists())
	{
		portaudio::System::terminate();
	}
}

auto PortAudioProducer::getInputDevices() -> std::list<DeviceInfo>
{
	std::list<DeviceInfo> deviceList;

	for (portaudio::System::DeviceIterator i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i)
	{

		if (i->isFullDuplexDevice() || i->isInputOnlyDevice())
		{
			DeviceInfo deviceInfo(&(*i), this->settings->getAudioInputDevice() == i->index());
			deviceList.push_back(deviceInfo);
		}

	}
	return deviceList;
}

auto PortAudioProducer::getSelectedInputDevice() -> DeviceInfo
{
	try
	{
		return DeviceInfo(&sys.deviceByIndex(this->settings->getAudioInputDevice()), true);
	}
	catch (portaudio::PaException& e)
	{
		g_message("PortAudioProducer: Selected input device not found - fallback to default input device\nCaused by: %s", e.what());
		return DeviceInfo(&sys.defaultInputDevice(), true);
	}
}

auto PortAudioProducer::isRecording() -> bool
{
	return this->inputStream != nullptr && this->inputStream->isActive();
}

auto PortAudioProducer::startRecording() -> bool
{
	// Check if there already is a recording
	if (this->inputStream != nullptr)
	{
		return false;
	}

	// Get the device information of our input device
	portaudio::Device* device = nullptr;
	try
	{
		device = &sys.deviceByIndex(getSelectedInputDevice().getIndex());
	}
	catch (portaudio::PaException& e)
	{
		g_message("PortAudioProducer: Unable to find selected input device");
		return false;
	}

	// Restrict recording channels to 2 as playback devices should have 2 channels at least
	this->inputChannels = std::min(2, device->maxInputChannels());
	portaudio::DirectionSpecificStreamParameters inParams(*device, this->inputChannels, portaudio::FLOAT32, true, device->defaultLowInputLatency(), nullptr);
	portaudio::StreamParameters params(inParams, portaudio::DirectionSpecificStreamParameters::null(), this->settings->getAudioSampleRate(), this->framesPerBuffer, paNoFlag);

	this->audioQueue->setAudioAttributes(this->settings->getAudioSampleRate(), static_cast<unsigned int>(this->inputChannels));

	// Specify the callback used for buffering the recorded data
	try
	{
		this->inputStream = new portaudio::MemFunCallbackStream<PortAudioProducer>(params, *this, &PortAudioProducer::recordCallback);
	}
	catch (portaudio::PaException& e)
	{
		g_message("PortAudioProducer: Unable to open stream");
		return false;
	}

	// Start the recording
	try
	{
		this->inputStream->start();
	}
	catch (portaudio::PaException& e)
	{
		g_message("PortAudioProducer: Unable to start stream");
		return false;
	}

	return true;
}

auto PortAudioProducer::recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                                       const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
        -> int
{
	if (statusFlags)
	{
		g_message("PortAudioProducer: statusFlag: %s", std::to_string(statusFlags).c_str());
	}

	if (inputBuffer != nullptr)
	{
		size_t providedFrames = framesPerBuffer * this->inputChannels;

		this->audioQueue->push(static_cast<float*>(const_cast<void*>(inputBuffer)), providedFrames);
	}
	return paContinue;
}

void PortAudioProducer::stopRecording()
{
	// Stop the recording
	if (this->inputStream != nullptr)
	{
		try
		{
			if (this->inputStream->isActive())
			{
				this->inputStream->stop();
			}
			if (this->inputStream->isOpen())
			{
				this->inputStream->close();
			}
		}
		catch (portaudio::PaException& e)
		{
			g_message("PortAudioProducer: Closing stream failed");
		}
	}

	// Notify the consumer at the other side that ther will be no more data
	this->audioQueue->signalEndOfStream();

	// Allow new recording by removing the old one
	delete this->inputStream;
	this->inputStream = nullptr;
}
