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
    DeviceId() = default;
    explicit DeviceId(const void* id): id(id) {}
    void clear() { id = nullptr; }
    operator bool() const { return id != nullptr; }

private:
    const void* id = nullptr;
};
