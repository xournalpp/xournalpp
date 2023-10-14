/*
 * Xournal++
 *
 * Device identifier
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

struct DeviceId {
    constexpr DeviceId() = default;
    explicit constexpr DeviceId(const void* id): id(id) {}
    constexpr void reset(const void* id = nullptr) { this->id = id; }
    explicit constexpr operator bool() const { return id != nullptr; }
    constexpr bool operator==(const DeviceId& o) const { return id == o.id; }
    constexpr bool operator!=(const DeviceId& o) const { return !(*this == o); }

private:
    const void* id = nullptr;
};
