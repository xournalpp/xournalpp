/*
 * Xournal++
 *
 * Element that is audio enabled
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include "Element.h"     // for Element, ElementType
#include "filesystem.h"  // for path

class ObjectInputStream;
class ObjectOutputStream;


class AudioElement: public Element {
protected:
    AudioElement(ElementType type);

public:
    ~AudioElement() override;

    void setTimestamp(size_t timestamp);
    size_t getTimestamp() const;

    void setAudioFilename(fs::path fn);
    auto getAudioFilename() const -> fs::path const&;

    /**
     * @brief Get the squared distance between a point and the object
     * @param p The point
     * @param veryClose Lower bound for the distance
     * @param toFar Upper bound for the distance
     * @return The squared distance, unless this distance is bigger than toFar (in this case, returns toFar) or the
     * distance is smaller than veryClose (returns veryClose)
     *
     * In practice, veryClose and toFar is used to limit the computation time.
     */
    virtual double isPointNearby(double x, double y, double veryClose, double toFar) const = 0;

protected:
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    void cloneAudioData(const AudioElement* other);

private:
    // Stroke timestamp, to match it to the audio stream
    size_t timestamp = 0;
    fs::path audioFilename{};
};
