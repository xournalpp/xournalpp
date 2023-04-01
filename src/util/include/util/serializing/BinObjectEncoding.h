/*
 * Xournal++
 *
 * Binary encoded serialized stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ObjectEncoding.h"

class BinObjectEncoding: public ObjectEncoding {
public:
    BinObjectEncoding();
    ~BinObjectEncoding() override;

    void addData(const void* data, size_t len) override;
};
