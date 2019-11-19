#include "XmlAudioNode.h"

#include <utility>

XmlAudioNode::XmlAudioNode(const char* tag)
		: XmlNode(tag),
		audioFilename("")
{
}

XmlAudioNode::~XmlAudioNode() = default;

auto XmlAudioNode::getAudioFilename() -> string
{
	return this->audioFilename;
}

void XmlAudioNode::setAudioFilename(string filename)
{
	this->audioFilename = std::move(filename);
}