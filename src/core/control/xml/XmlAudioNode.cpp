#include "XmlAudioNode.h"

#include <utility>  // for move

#include "control/xml/XmlNode.h"  // for XmlNode
#include "util/StringUtils.h"     // for StaticStringView

XmlAudioNode::XmlAudioNode(StringUtils::StaticStringView tag): XmlNode(tag), audioFilepath{} {}

XmlAudioNode::~XmlAudioNode() = default;

auto XmlAudioNode::getAudioFilepath() -> fs::path { return this->audioFilepath; }

void XmlAudioNode::setAudioFilepath(fs::path filepath) { this->audioFilepath = std::move(filepath); }
