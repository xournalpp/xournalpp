#pragma once

#include "AbstractXmlNode.h"
#include "util/OutputStream.h" 
#include <string>

class OutputStream;

class XmlPreserializedNode : public AbstractXmlNode {
public:
    explicit XmlPreserializedNode(std::string rawXml) : rawXmlString(std::move(rawXml)) {}
    virtual ~XmlPreserializedNode() = default;

    void writeOut(OutputStream* out) override;

private:
    std::string rawXmlString;
};