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

#include <array>    // for array
#include <cstddef>  // for size_t

#include <glib.h>  // for gint

class OutputStream;

class XmlBase64Writer {
public:
    explicit XmlBase64Writer(OutputStream* out);

    void write(const unsigned char* data, std::size_t length);
    void finish();

private:
    static constexpr std::size_t CHUNK_SIZE = std::size_t{3} * 4096;

    OutputStream* out;
    std::array<char, (CHUNK_SIZE / 3 + 1) * 4 + 4> buffer{};
    gint state = 0;
    gint save = 0;
};
