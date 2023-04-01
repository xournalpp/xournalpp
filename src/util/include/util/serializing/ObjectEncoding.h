/*
 * Xournal++
 *
 * Encoding for serialized streams
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>
#include <string_view>

class ObjectEncoding {
public:
    ObjectEncoding();
    virtual ~ObjectEncoding();

public:
    void addStr(const std::string_view& str);
    virtual void addData(const void* data, size_t len) = 0;

    std::vector<std::byte> const& getData();
public:
    std::vector<std::byte> data;
};
