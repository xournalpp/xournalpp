#include "PortAudioConsumer.h"
#include "AudioPlayer.h"

PortAudioConsumer::PortAudioConsumer(AudioPlayer* audioPlayer, AudioQueue<float>* audioQueue) : sys(portaudio::System::instance()), audioPlayer(audioPlayer), audioQueue(audioQueue)
{
}

PortAudioConsumer::~PortAudioConsumer()
{
	if (portaudio::System::exists())
	{
		portaudio::System::terminate();
	}
}

std::list<DeviceInfo> PortAudioConsumer::getOutputDevices()
{
	std::list<DeviceInfo> deviceList;

	for (portaudio::System::DeviceIterator i = this->sys.devicesBegin(); i != sys.devicesEnd(); ++i)
	{

		if (i->isFullDuplexDevice() || i->isOutputOnlyDevice())
		{
			DeviceInfo deviceInfo(&(*i), this->audioPlayer->getSettings()->getAudioOutputDevice() == i->index());
			deviceList.push_back(deviceInfo);
		}

	}
	return deviceList;
}

const DeviceInfo PortAudioConsumer::getSelectedOutputDevice()
{
	try
	{
		return DeviceInfo(&sys.deviceByIndex(this->audioPlayer->getSettings()->getAudioOutputDevice()), true);
	}
	catch (portaudio::PaException& e)
	{
		g_warning("PortAudioConsumer: Selected output device was not found - fallback to default output device\nCaused by: %s", e.what());
		return DeviceInfo(&sys.defaultOutputDevice(), true);
	}
}

bool PortAudioConsumer::isPlaying()
{
	return this->outputStream != nullptr && this->outputStream->isActive();
}

bool PortAudioConsumer::startPlaying()
{
	// Abort a playback stream if one is currently active
	if (this->outputStream != nullptr && this->outputStream->isActive())
	{
		this->outputStream->abort();
	}

	double sampleRate;
	unsigned int channels;
	this->audioQueue->getAudioAttributes(sampleRate, channels);

	if (sampleRate == -1)
	{
		g_warning("PortAudioConsumer: Timing issue - Sample rate requested before known");
		return false;
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
		return false;
	}

	if ((unsigned int) device->maxOutputChannels() < channels)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("Output device has not enough channels to play audio file. (Requires at least 2 channels)");
		return false;
	}

	this->outputChannels = channels;
	portaudio::DirectionSpecificStreamParameters outParams(*device, channels, portaudio::FLOAT32, true, device->defaultLowOutputLatency(), nullptr);
	portaudio::StreamParameters params(portaudio::DirectionSpecificStreamParameters::null(), outParams, sampleRate, this->framesPerBuffer, paNoFlag);

	try
	{
		this->outputStream = new portaudio::MemFunCallbackStream<PortAudioConsumer>(params, *this, &PortAudioConsumer::playCallback);
	}
	catch (portaudio::PaException& e)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("PortAudioConsumer: Unable to open stream to device\nCaused by: %s", e.what());
		return false;
	}
	// Start the recording
	try
	{
		this->outputStream->start();
	}
	catch (portaudio::PaException& e)
	{
		this->audioQueue->signalEndOfStream();
		g_warning("PortAudioConsumer: Unable to start stream\nCaused by: %s", e.what());
		return false;
	}
	return true;
}

int PortAudioConsumer::playCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
									PaStreamCallbackFlags statusFlags)
{
	if (statusFlags)
	{
		g_warning("PortAudioConsumer: PortAudio reported a stream warning: %s", std::to_string(statusFlags).c_str());
	}

	if (outputBuffer != nullptr)
	{
		size_t outputBufferLength;
		this->audioQueue->pop(((float*) outputBuffer), outputBufferLength, framesPerBuffer * this->outputChannels);

		// Fill buffer to requested length if necessary

		if (outputBufferLength < framesPerBuffer * this->outputChannels)
		{
			// Show frame underflow warning if there are not enough samples and the stream is not yet finished
			if (!this->audioQueue->hasStreamEnded())
			{
				g_warning("PortAudioConsumer: Not enough audio samples available to fill requested frame");
			}

			auto outputBufferImpl = (float*) outputBuffer;

			if (outputBufferLength > this->outputChannels)
			{
				// If there is previous audio data use this data to ramp down the audio samples
				for (auto i = outputBufferLength; i < framesPerBuffer * this->outputChannels; ++i)
				{
					outputBufferImpl[i] = outputBufferImpl[i - this->outputChannels] / 2;
				}
			}
			else
			{
				// If there is no data that could be used to ramp down just output silence
				for (auto i = outputBufferLength; i < framesPerBuffer * this->outputChannels; ++i)
				{
					outputBufferImpl[i] = 0;
				}
			}
		}

		// Continue playback if there is still data available
		if (this->audioQueue->hasStreamEnded() && this->audioQueue->empty())
		{
			this->audioPlayer->disableAudioPlaybackButtons();
			return paComplete;
		}
		else
		{
			return paContinue;
		}
	}

	// The output buffer is no longer available - Abort!
	this->audioQueue->signalEndOfStream();
	this->audioPlayer->disableAudioPlaybackButtons();
	return paAbort;
}

void PortAudioConsumer::stopPlaying()
{
	// Stop the playback
	if (this->outputStream != nullptr)
	{
		try
		{
			if (this->outputStream->isActive())
			{
				this->outputStream->stop();
			}
		}
		catch (portaudio::PaException& e)
		{
			/*
			 * We try closing the stream but this->outputStream might be an invalid object at this time if the stream was previously closed by the backend.
			 * Just ignore this as the stream is closed either way.
			 */
		}
	}
}
