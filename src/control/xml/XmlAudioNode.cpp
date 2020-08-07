#include "XmlAudioNode.h"

#include <utility>

XmlAudioNode::XmlAudioNode(const char* tag): XmlNode(tag), audioFilepath{} {}

XmlAudioNode::~XmlAudioNode() = default;

auto XmlAudioNode::getAudioFilepath() -> fs::path { return this->audioFilepath; }

void XmlAudioNode::setAudioFilepath(fs::path filepath) { this->audioFilepath = std::move(filepath); }
