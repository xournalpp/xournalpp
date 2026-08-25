/*
 * Xournal++
 *
 * Audio attached to an element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include "util/serializing/Serializable.h"  // for Serializable

#include "filesystem.h"  // for path

class ObjectInputStream;
class ObjectOutputStream;

class AudioContent: public Serializable {
public:
    AudioContent() = default;
    AudioContent(AudioContent&&) = default;
    AudioContent(const AudioContent&) = default;
    AudioContent& operator=(AudioContent&&) = default;
    AudioContent& operator=(const AudioContent&) = default;

    ~AudioContent() override = default;
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    inline void setTimestamp(size_t ts) { timestamp = ts; }
    inline size_t getTimestamp() const { return timestamp; }

    inline void setAudioFilename(fs::path fn) { audioFilename = std::move(fn); }
    inline auto getAudioFilename() const -> fs::path const& { return audioFilename; }

    size_t timestamp = 0;  ///< Timestamp, to match it to the audio stream
    fs::path audioFilename{};
};
