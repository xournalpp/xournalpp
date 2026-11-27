/*
 * Xournal++
 *
 * XML Base64 writer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>  // for min
#include <array>      // for array
#include <cstddef>    // for size_t

#include <glib.h>  // for FALSE, g_base64_encode_close, g_base64_encode_step, gint, gsize

#include "util/OutputStream.h"  // for OutputStream

class XmlBase64Writer {
public:
    explicit XmlBase64Writer(OutputStream* out): out(out) {}

    void write(const unsigned char* data, std::size_t length) {
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

    void finish() {
        std::array<char, 8> tail{};
        const gsize encoded = g_base64_encode_close(FALSE, tail.data(), &this->state, &this->save);
        if (encoded > 0) {
            this->out->write(tail.data(), encoded);
        }
    }

private:
    static constexpr std::size_t CHUNK_SIZE = std::size_t{3} * 4096;

    OutputStream* out;
    std::array<char, (CHUNK_SIZE / 3 + 1) * 4 + 4> buffer{};
    gint state = 0;
    gint save = 0;
};
