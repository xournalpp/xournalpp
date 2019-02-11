#include "XmlAudioNode.h"

XmlAudioNode::XmlAudioNode(const char* tag)
		: XmlNode(tag),
		audioFilename("")
{
	XOJ_INIT_TYPE(XmlAudioNode);
}

XmlAudioNode::~XmlAudioNode()
{
	XOJ_CHECK_TYPE(XmlAudioNode);

	XOJ_RELEASE_TYPE(XmlAudioNode);
}

string XmlAudioNode::getAudioFilename()
{
	return this->audioFilename;
}

void XmlAudioNode::setAudioFilename(string filename)
{
	this->audioFilename = filename;
}