/*
 * Xournal++
 *
 *
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

/**
 * Empty base class for overlays: anything that is drawn over a document, but not in the document
 *  (e.g. active tools, selections, search results highlighting...).
 *
 * This class is only used for pointer comparison.
 */
class OverlayBase {
public:
    virtual ~OverlayBase() = default;
};
