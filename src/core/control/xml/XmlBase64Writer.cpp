#include "XmlBase64Writer.h"

#include <algorithm>  // for min

#include <glib.h>  // for FALSE, g_base64_encode_close, g_base64_encode_step, gsize

#include "util/OutputStream.h"  // for OutputStream

XmlBase64Writer::XmlBase64Writer(OutputStream* out): out(out) {}

void XmlBase64Writer::write(const unsigned char* data, std::size_t length) {
    while (length > 0) {
        const std::size_t n = std::min(length, CHUNK_SIZE);
        const gsize encoded = g_base64_encode_step(data, n, FALSE, this->buffer.data(), &this->state, &this->save);
        if (encoded > 0) {
            this->out->write(this->buffer.data(), encoded);
        }
        data += n;
        length -= n;
    }
}

void XmlBase64Writer::finish() {
    std::array<char, 8> tail{};
    const gsize encoded = g_base64_encode_close(FALSE, tail.data(), &this->state, &this->save);
    if (encoded > 0) {
        this->out->write(tail.data(), encoded);
    }
}
