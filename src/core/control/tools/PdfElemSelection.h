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

class PdfElemSelection {
public:
    PdfElemSelection(double x, double y, XojPageView* view);
    virtual ~PdfElemSelection();

public:
    virtual bool finalize(PageRef page);
    virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
    virtual void currentPos(double x, double y);
    virtual bool contains(double x, double y);

    bool selectHeadTailTextRegion();
    void selectFinally();
    void selectHeadTailFinally();
    void clearSelection();

    const std::vector<XojPdfRectangle>& getSelectedTextRects() const;
    const std::string& getSelectedText() const;
    XojPageView* getPageView() const;

    void setIsFinalized(const bool finalized);
    bool getIsFinalized() const;

    void setIsFinished(const bool finished);
    bool getIsFinished() const;

    void doublePress();
    void triplePress();

    int getSelectionPageNr() const;
    void setToolType(ToolType toolType);


private:
    XojPageView* view = nullptr;
    XojPdfPageSPtr pdf;

    std::vector<XojPdfRectangle> selectedTextRects;
    std::string selectedText;
    cairo_region_t* selectedTextRegion = nullptr;

    ToolType toolType;

    XojPdfPageSelectionStyle selectionStyle = XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_GLYPH;
    long unsigned int selectionPageNr = npos;

    double sx;
    double sy;
    double ex;
    double ey;

    bool isFinalized;
    bool isFinished;
};
