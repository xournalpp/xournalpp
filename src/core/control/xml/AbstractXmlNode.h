#pragma once

class OutputStream;  // Forward declaration

class AbstractXmlNode {
public:
    virtual ~AbstractXmlNode() = default;

    virtual void writeOut(OutputStream* out) = 0;
};
