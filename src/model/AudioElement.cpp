#include "AudioElement.h"

AudioElement::AudioElement(ElementType type)
 : Element(type)
{
	XOJ_INIT_TYPE(AudioElement);
}

AudioElement::~AudioElement()
{

	XOJ_CHECK_TYPE(AudioElement);

	this->timestamp = 0;

	XOJ_RELEASE_TYPE(AudioElement);
}

void AudioElement::setAudioFilename(string fn)
{
	XOJ_CHECK_TYPE(AudioElement);

	this->audioFilename = fn;
}

string AudioElement::getAudioFilename() const
{
	XOJ_CHECK_TYPE(AudioElement);

	return this->audioFilename;
}

void AudioElement::setTimestamp(size_t timestamp)
{
	XOJ_CHECK_TYPE(AudioElement);

	this->timestamp = timestamp;
}

size_t AudioElement::getTimestamp() const
{
	XOJ_CHECK_TYPE(AudioElement);

	return this->timestamp;
}

void AudioElement::serializeAudioElement(ObjectOutputStream& out)
{
	XOJ_CHECK_TYPE(AudioElement);

	out.writeObject("AudioElement");

	serializeElement(out);

	out.writeString(this->audioFilename);
	out.writeSizeT(this->timestamp);

	out.endObject();
}

void AudioElement::readSerializedAudioElement(ObjectInputStream& in)
{
	XOJ_CHECK_TYPE(AudioElement);

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
