/*
 * Xournal++
 *
 * Class for render and repaint pages
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class XojPageView;
class XournalView;

class RepaintHandler {
public:
    RepaintHandler(XournalView* xournal);
    virtual ~RepaintHandler();

public:
    /**
     * Repaint a page
     */
    void repaintPage(const XojPageView* view);

    /**
     * Repaint a page area, coordinates are in view coordinates
     */
    void repaintPageArea(const XojPageView* view, int x1, int y1, int x2, int y2);

    /**
     * Repaints the page border (at least)
     */
    void repaintPageBorder(const XojPageView* view);

private:
    XournalView* xournal;
};
