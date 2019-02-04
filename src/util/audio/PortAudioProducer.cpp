#include "PortAudioProducer.h"

PortAudioProducer::PortAudioProducer(Settings* settings, AudioQueue* audioQueue)
		: sys(portaudio::System::instance()),
		  settings(settings),
		  audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(PortAudioProducer);
}

PortAudioProducer::~PortAudioProducer()
{
	XOJ_CHECK_TYPE(PortAudioProducer);

	if (portaudio::System::exists())
	{
		portaudio::System::terminate();
	}

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
			DeviceInfo deviceInfo(&(*i), this->settings->getAudioInputDevice() == i->index());
			deviceList.push_back(deviceInfo);
		}

	}
	return deviceList;
}

const DeviceInfo PortAudioProducer::getSelectedInputDevice()
{
	XOJ_CHECK_TYPE(PortAudioProducer);

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

bool PortAudioProducer::isRecording()
{
	XOJ_CHECK_TYPE(PortAudioProducer);

	return this->inputStream != nullptr && this->inputStream->isActive();
}

bool PortAudioProducer::startRecording()
{
	XOJ_CHECK_TYPE(PortAudioProducer);

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
	portaudio::DirectionSpecificStreamParameters inParams(*device, this->inputChannels, portaudio::INT32, true, device->defaultLowInputLatency(), nullptr);
	portaudio::StreamParameters params(inParams, portaudio::DirectionSpecificStreamParameters::null(), this->settings->getAudioSampleRate(), this->framesPerBuffer, paNoFlag);

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

int PortAudioProducer::recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
									  const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
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

		this->audioQueue->push((int*) inputBuffer, providedFrames);
	}
	return paContinue;
}

void PortAudioProducer::stopRecording()
{
	XOJ_CHECK_TYPE(PortAudioProducer);

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
