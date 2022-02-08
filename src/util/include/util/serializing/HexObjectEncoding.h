/*
 * Xournal++
 *
 * Hex encoded serialized stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ObjectEncoding.h"

class HexObjectEncoding: public ObjectEncoding {
public:
    HexObjectEncoding();
    ~HexObjectEncoding() override;

public:
    void addData(const void* data, int len) override;

private:
};
