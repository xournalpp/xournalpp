/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XmlNode.h"
#include "XmlAudioNode.h"

class XmlTextNode : public XmlAudioNode
{
public:
	XmlTextNode(const char* tag, std::string text);
	explicit XmlTextNode(const char* tag);
	~XmlTextNode() override = default;

public:
	void setText(std::string text);

	void writeOut(OutputStream* out) override;

private:
	std::string text;
};
