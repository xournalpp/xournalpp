#include "AudioElement.h"

AudioElement::AudioElement(ElementType type)
 : Element(type)
{
}

AudioElement::~AudioElement()
{
	this->timestamp = 0;
}

void AudioElement::setAudioFilename(string fn)
{
	this->audioFilename = fn;
}

string AudioElement::getAudioFilename() const
{
	return this->audioFilename;
}

void AudioElement::setTimestamp(size_t timestamp)
{
	this->timestamp = timestamp;
}

size_t AudioElement::getTimestamp() const
{
	return this->timestamp;
}

void AudioElement::serializeAudioElement(ObjectOutputStream& out)
{
	out.writeObject("AudioElement");

	serializeElement(out);

	out.writeString(this->audioFilename);
	out.writeSizeT(this->timestamp);

	out.endObject();
}

void AudioElement::readSerializedAudioElement(ObjectInputStream& in)
{
	in.readObject("AudioElement");

	readSerializedElement(in);

	this->audioFilename = in.readString();
	this->timestamp = in.readSizeT();

	in.endObject();
}

void AudioElement::cloneAudioData(const AudioElement* other)
{
	setAudioFilename(other->getAudioFilename());
	setTimestamp(other->getTimestamp());
}
