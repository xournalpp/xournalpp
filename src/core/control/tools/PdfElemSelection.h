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

#include <string>
#include <vector>

#include "control/ToolEnums.h"
#include "gui/PageView.h"
#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/PageRef.h"
#include "pdf/base/XojPdfPage.h"

/// Represents elements selected from a PDF page, such as text.
class PdfElemSelection {
public:
    PdfElemSelection(double x, double y, XojPageView* view);
    PdfElemSelection& operator=(const PdfElemSelection&) = delete;
    PdfElemSelection(const PdfElemSelection&) = delete;
    PdfElemSelection& operator=(PdfElemSelection&&) = default;
    PdfElemSelection(PdfElemSelection&&) = default;
    virtual ~PdfElemSelection();

public:
    /// Calls finalizeSelection() and then repaints the page.
    bool finalizeSelectionAndRepaint(XojPdfPageSelectionStyle style);

    /// Sets the final selection bounds using the provided selection style.
    /// Returns true iff there is text contained within the selection bounds.
    bool finalizeSelection(XojPdfPageSelectionStyle style);

    /// Render the selection visuals with the given style
    void paint(cairo_t* cr, XojPdfPageSelectionStyle style);

    /// Update the (unfinalized) selection bounds with the given
    /// style.
    void currentPos(double x, double y, XojPdfPageSelectionStyle style);

    /// If the selection is a rectangle, returns true iff the given point is
    /// contained in the selection. Returns false on text selection.
    bool contains(double x, double y);

    const std::vector<XojPdfRectangle>& getSelectedTextRects() const;

    /// Returns the text contained in the selection region.
    const std::string& getSelectedText() const;

    XojPageView* getPageView() const;

    /// Returns true iff the final selections bounds are known.
    bool isFinalized() const;

    /// Trigger double press selection action, with page repaint.
    void doublePress();

    /// Trigger triple press selection action, with page repaint.
    void triplePress();

    uint64_t getSelectionPageNr() const;
    void setToolType(ToolType toolType);

    /// Returns the selection style corresponding to the given tool type.
    static XojPdfPageSelectionStyle selectionStyleForToolType(ToolType type);

private:
    /// Assigns the selected text region to the current selection bounds.
    bool selectTextRegion(XojPdfPageSelectionStyle style);

    XojPageView* view;
    XojPdfPageSPtr pdf;

    /// The rectangles corresponding to the lines of selected text.
    std::vector<XojPdfRectangle> selectedTextRects;

    /// The text content of the selection.
    std::string selectedText;

    /// The area containing the selected text. Used for rendering.
    cairo_region_t* selectedTextRegion = nullptr;

    /// The PDF selection tool used for the selection.
    ToolType toolType;

    long unsigned int selectionPageNr = npos;

    /// The selection bounds. Note that this does not necessarily correspond to
    /// a rectangle--it may also indicate start and end positions of a linear
    /// text selection.
    XojPdfRectangle bounds;

    bool finalized;
};
