/*
 * Xournal++
 *
 * Geometric target confirmation and whole-stroke erasure
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include "control/settings/ScribbleSettings.h"
#include "model/PageRef.h"

class DeleteUndoAction;
class Document;
class Range;
class Stroke;

class ScribbleEraser {
public:
    // Returns one already-applied undo action, or leaves the document unchanged on rejection.
    // Repaint and history notifications remain the caller's responsibility, outside the document lock.
    static auto erase(Document& document, const PageRef& page, const Stroke& gesture, Range& changedRange,
                      ScribbleSettings settings = {}) -> std::unique_ptr<DeleteUndoAction>;
};
