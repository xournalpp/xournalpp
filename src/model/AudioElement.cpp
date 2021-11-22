#include "AudioElement.h"

#include <utility>

AudioElement::AudioElement(ElementType type): Element(type) {}

AudioElement::~AudioElement() { this->timestamp = 0; }

void AudioElement::setAudioFilename(std::string fn) { this->audioFilename = std::move(fn); }

auto AudioElement::getAudioFilename() const -> std::string { return this->audioFilename; }

void AudioElement::setTimestamp(size_t timestamp) { this->timestamp = timestamp; }

auto AudioElement::getTimestamp() const -> size_t { return this->timestamp; }

void AudioElement::serialize(ObjectOutputStream& out) const {
    out.writeObject("AudioElement");

    this->Element::serialize(out);

    out.writeString(this->audioFilename);
    out.writeSizeT(this->timestamp);

    out.endObject();
}

void AudioElement::readSerialized(ObjectInputStream& in) {
    in.readObject("AudioElement");

    this->Element::readSerialized(in);

    this->audioFilename = in.readString();
    this->timestamp = in.readSizeT();

    in.endObject();
}

void AudioElement::cloneAudioData(const AudioElement* other) {
    setAudioFilename(other->getAudioFilename());
    setTimestamp(other->getTimestamp());
}
