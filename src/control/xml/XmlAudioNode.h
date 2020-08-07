/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "XmlNode.h"

class XmlAudioNode: public XmlNode {
public:
    XmlAudioNode(const char* tag);
    virtual ~XmlAudioNode();

private:
    XmlAudioNode(const XmlAudioNode& node);
    void operator=(const XmlAudioNode& node);

public:
    fs::path getAudioFilepath();
    void setAudioFilepath(fs::path filepath);

private:
    fs::path audioFilepath;
};
