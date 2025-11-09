#include "XmlPreserializedNode.h"

void XmlPreserializedNode::writeOut(OutputStream* out) {
    out->write(this->rawXmlString); 
}