#include "VorbisProducer.h"

VorbisProducer::VorbisProducer(AudioQueue<float>* audioQueue) : audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(VorbisProducer);
}

VorbisProducer::~VorbisProducer()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	XOJ_RELEASE_TYPE(VorbisProducer);
}

void VorbisProducer::start(std::string filename, const DeviceInfo& outputDevice, unsigned int timestamp)
{
	XOJ_CHECK_TYPE(VorbisProducer);

	this->inputFile = std::ifstream(filename, std::ios::binary);
	try
	{
		inputFile.exceptions(this->inputFile.failbit);
	}
	catch (const std::ios_base::failure& e)
	{
		g_warning("VorbisProducer: input file \"%s\" could not be opened", filename.c_str());
		return;
	}

	const ov_callbacks callbacks{read, seek, nullptr, tell};
	if (ov_open_callbacks(&(this->inputFile), &vf, nullptr, 0, callbacks) < 0)
	{
		g_warning("VorbisProducer: input file \"%s\" does not contain a valid Ogg bitstream", filename.c_str());
		return;
	}

	this->vi = ov_info(&vf, -1);

	//TODO implement seeking (this is hard since we need to get the frame offset)

	this->producerThread = new std::thread(
			[&, filename, timestamp]
			{
				long numSamples = 1;
				int current_section;
				auto sampleBuffer = new float[1024 * this->vi->channels];

				while (!this->stopProducer && numSamples > 0 && !this->audioQueue->hasStreamEnded())
				{

					float** buffer;
					numSamples = ov_read_float(&(this->vf), &buffer, 2048, &current_section);
					if (numSamples < 0)
					{
						g_warning("VorbisProducer: An error occured while decoding the bitstream: %ld", numSamples);
						return;
					}

					while (this->audioQueue->size() > 8192 && !this->audioQueue->hasStreamEnded())
					{
						std::this_thread::sleep_for(std::chrono::microseconds(100));
					}

					for (long i = 0; i < numSamples / this->vi->channels; i++)
					{
						for (int j = 0; j < this->vi->channels; j++)
						{
							sampleBuffer[i * this->vi->channels + j] = buffer[j][i];
						}
					}

					this->audioQueue->push(sampleBuffer, static_cast<unsigned long>(numSamples));
				}
				this->audioQueue->signalEndOfStream();

				delete[] sampleBuffer;
				sampleBuffer = nullptr;

				ov_clear(&(this->vf));
			});
}

vorbis_info* VorbisProducer::getSignalInformation()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	return this->vi;
}

void VorbisProducer::abort()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	this->stopProducer = true;
	// Wait for producer to finish
	stop();
	this->stopProducer = false;

}

void VorbisProducer::stop()
{
	XOJ_CHECK_TYPE(VorbisProducer);

	// Wait for producer to finish
	if (this->producerThread && this->producerThread->joinable())
	{
		this->producerThread->join();
	}
}


size_t VorbisProducer::read(void* buffer, size_t elementSize, size_t elementCount, void* dataSource) {
	std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
	try
	{
	stream.read(static_cast<char*>(buffer), elementCount * elementSize);
		const std::streamsize bytesRead = stream.gcount();
		stream.clear();
		return static_cast<size_t>(bytesRead / elementSize);
	}
	catch (const std::ios_base::failure& e)
	{
		return 0;
	}

}

int VorbisProducer::seek(void* dataSource, ogg_int64_t offset, int origin) {
	static const std::vector<std::ios_base::seekdir> seekDirections
	{
			std::ios_base::beg, std::ios_base::cur, std::ios_base::end
	};

	std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
	stream.seekg(offset, seekDirections.at(static_cast<unsigned long>(origin)));
	stream.clear();
	return 0;
}

long VorbisProducer::tell(void* dataSource) {
	std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
	const auto position = stream.tellg();
	return static_cast<long>(position);
}