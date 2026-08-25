#include "AudioContent.h"

#include "util/StringUtils.h"                     // for char_cast
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "util/utf8_view.h"

void AudioContent::serialize(ObjectOutputStream& out) const {
    out.writeObject("AudioContent");

    out.writeString(char_cast(this->audioFilename.u8string()));
    out.writeSizeT(this->timestamp);

    out.endObject();
}

void AudioContent::readSerialized(ObjectInputStream& in) {
    in.readObject("AudioContent");

    this->audioFilename = fs::path(xoj::util::utf8(in.readString()));
    this->timestamp = in.readSizeT();

    in.endObject();
}
