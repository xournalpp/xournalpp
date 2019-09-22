#include "XmlAudioNode.h"

XmlAudioNode::XmlAudioNode(const char* tag)
		: XmlNode(tag),
		audioFilename("")
{
}

XmlAudioNode::~XmlAudioNode()
{
}

string XmlAudioNode::getAudioFilename()
{
	return this->audioFilename;
}

void XmlAudioNode::setAudioFilename(string filename)
{
	this->audioFilename = filename;
}