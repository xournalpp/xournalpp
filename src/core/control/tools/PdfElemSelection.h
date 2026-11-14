/*
 * Xournal++
 *
 * Tool for selecting PDF content
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string
#include <vector>   // for vector

#include <cairo.h>  // for cairo_region_t, cairo_t

#include "model/OverlayBase.h"
#include "model/PageRef.h"
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSelectionStyle, XojPdfRec...
#include "util/Color.h"
#include "util/DispatchPool.h"
#include "util/Util.h"                // for npos
#include "util/raii/CairoWrappers.h"  // for CairoRegionSPtr

class Range;
class Control;
class ToolHandler;

namespace xoj::view {
class PdfElementSelectionView;
};

/// Represents elements selected from a PDF page, such as text.
class PdfElemSelection: public OverlayBase {
public:
    PdfElemSelection(double x, double y, ToolHandler& th, PageRef page, Control* control);
    PdfElemSelection& operator=(const PdfElemSelection&) = delete;
    PdfElemSelection(const PdfElemSelection&) = delete;
    PdfElemSelection& operator=(PdfElemSelection&&) = default;
    PdfElemSelection(PdfElemSelection&&) = default;
    virtual ~PdfElemSelection();

public:
    /// Calls finalizeSelection() and then repaints the page.
    bool finalizeSelectionAndRepaint();

    /// Sets the final selection bounds.
    /// Returns true iff there is text contained within the selection bounds.
    bool finalizeSelection();

    void changeStyle(XojPdfPageSelectionStyle style);

    /// Update the (unfinalized) selection bounds
    void currentPos(double x, double y);

    /// If the selection is a rectangle, returns true iff the given point is
    /// contained in the selection. Returns false on text selection.
    bool contains(double x, double y);

    const std::vector<XojPdfRectangle>& getSelectedTextRects() const;

    /// Returns the text contained in the selection region.
    const std::string& getSelectedText() const;

    /// Returns true iff the final selections bounds are known.
    bool isFinalized() const;

    const cairo_region_t* getSelectedRegion() const { return selectedTextRegion.get(); }

    inline const std::shared_ptr<xoj::util::DispatchPool<xoj::view::PdfElementSelectionView>>& getViewPool() const {
        return viewPool;
    }

    void strikethrough();  ///< Draw a stroke striking through the selected text
    void underline();      ///< Draw a stroke underlining the selected text
    void highlight();      ///< Draw a stroke highlighting the selected text
    bool copy();           ///< Copy the selected text to the clipboard. Returns true if some text was selected


    enum class MarkerStyle : uint8_t {
        POS_TEXT_BOTTOM = 0,
        POS_TEXT_MIDDLE,
        POS_TEXT_TOP,

        WIDTH_TEXT_LINE,
        WIDTH_TEXT_HEIGHT
    };

    void createStrokes(MarkerStyle position, MarkerStyle width, int markerOpacity);

private:
    Range getRegionBbox() const;

    XojPdfPageSPtr pdf;
    PageRef xoppPage;
    Control* control;

    /// The rectangles corresponding to the lines of selected text.
    std::vector<XojPdfRectangle> selectedTextRects;

    /// The text content of the selection.
    std::string selectedText;

    /// The area containing the selected text. Used for rendering.
    xoj::util::CairoRegionSPtr selectedTextRegion;

    size_t selectionPageNr = npos;

    /// The selection bounds. Note that this does not necessarily correspond to a rectangle--it may also indicate start
    /// and end positions of a linear text selection.
    XojPdfRectangle bounds;

    XojPdfPageSelectionStyle style;

    Color color;  ///< Used for strokes/highlighting
    uint8_t highlighterAlpha;

    bool finalized;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::PdfElementSelectionView>> viewPool;
};
