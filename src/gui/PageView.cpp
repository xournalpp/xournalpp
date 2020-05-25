#include "PageView.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>

#include <gdk/gdk.h>

#include "control/Control.h"
#include "control/SearchControl.h"
#include "control/jobs/BlockingJob.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/tools/ArrowHandler.h"
#include "control/tools/CircleHandler.h"
#include "control/tools/CoordinateSystemHandler.h"
#include "control/tools/EraseHandler.h"
#include "control/tools/ImageHandler.h"
#include "control/tools/InputHandler.h"
#include "control/tools/RectangleHandler.h"
#include "control/tools/RulerHandler.h"
#include "control/tools/Selection.h"
#include "control/tools/SplineHandler.h"
#include "control/tools/StrokeHandler.h"
#include "control/tools/VerticalToolHandler.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "undo/DeleteUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/TextBoxUndoAction.h"
#include "util/XojMsgBox.h"
#include "view/TextView.h"
#include "widgets/XournalWidget.h"

#include "PageViewFindObjectHelper.h"
#include "Range.h"
#include "Rectangle.h"
#include "RepaintHandler.h"
#include "TextEditor.h"
#include "XournalView.h"
#include "XournalppCursor.h"
#include "config-debug.h"
#include "config-features.h"
#include "config.h"
#include "i18n.h"
#include "pixbuf-utils.h"

XojPageView::XojPageView(XournalView* xournal, const PageRef& page) {
    this->page = page;
    this->registerListener(this->page);
    this->xournal = xournal;
    this->settings = xournal->getControl()->getSettings();

    g_mutex_init(&this->drawingMutex);

    g_mutex_init(&this->repaintRectMutex);

    // this does not have to be deleted afterwards:
    // (we need it for undo commands)
    this->oldtext = nullptr;

    this->eraser = new EraseHandler(xournal->getControl()->getUndoRedoHandler(), xournal->getControl()->getDocument(),
                                    this->page, xournal->getControl()->getToolHandler(), this);
}

XojPageView::~XojPageView() {
    // Unregister listener before destroying this handler
    this->unregisterListener();

    this->xournal->getControl()->getScheduler()->removePage(this);
    delete this->inputHandler;
    delete this->eraser;
    endText();
    deleteViewBuffer();
    delete this->search;
}

void XojPageView::setIsVisible(bool visible) {
    if (visible) {
        this->lastVisibleTime = 0;
    } else if (this->lastVisibleTime <= 0) {
        GTimeVal val;
        g_get_current_time(&val);
        this->lastVisibleTime = val.tv_sec;
    }
}

auto XojPageView::getLastVisibleTime() -> int {
    if (this->crBuffer == nullptr) {
        return -1;
    }

    return this->lastVisibleTime;
}

void XojPageView::deleteViewBuffer() {
    g_mutex_lock(&this->drawingMutex);
    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
    g_mutex_unlock(&this->drawingMutex);
}

auto XojPageView::containsPoint(int x, int y, bool local) const -> bool {
    if (!local) {
        bool leftOk = this->getX() <= x;
        bool rightOk = x <= this->getX() + this->getDisplayWidth();
        bool topOk = this->getY() <= y;
        bool bottomOk = y <= this->getY() + this->getDisplayHeight();

        return leftOk && rightOk && topOk && bottomOk;
    }


    return x >= 0 && y >= 0 && x <= this->getWidth() && y <= this->getHeight();
}

auto XojPageView::searchTextOnPage(string& text, int* occures, double* top) -> bool {
    if (this->search == nullptr) {
        if (text.empty()) {
            return true;
        }

        int pNr = this->page->getPdfPageNr();
        XojPdfPageSPtr pdf = nullptr;
        if (pNr != -1) {
            Document* doc = xournal->getControl()->getDocument();

            doc->lock();
            pdf = doc->getPdfPage(pNr);
            doc->unlock();
        }
        this->search = new SearchControl(page, pdf);
    }

    bool found = this->search->search(text, occures, top);

    repaintPage();

    return found;
}

void XojPageView::endText() {
    if (!this->textEditor) {
        return;
    }
    Text* txt = this->textEditor->getText();
    Layer* layer = this->page->getSelectedLayer();
    UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();

    // Text deleted
    if (txt->getText().empty()) {
        // old element
        int pos = layer->indexOf(txt);
        if (pos != -1) {
            auto eraseDeleteUndoAction = std::make_unique<DeleteUndoAction>(page, true);
            layer->removeElement(txt, false);
            eraseDeleteUndoAction->addElement(layer, txt, pos);
            undo->addUndoAction(std::move(eraseDeleteUndoAction));
        }
    } else {
        // new element
        if (layer->indexOf(txt) == -1) {
            undo->addUndoActionBefore(std::make_unique<InsertUndoAction>(page, layer, txt),
                                      this->textEditor->getFirstUndoAction());
            layer->addElement(txt);
            this->textEditor->textCopyed();
        }
        // or if the file was saved and reopened
        // and/or if we click away from the text window
        else {
            // TextUndoAction does not work because the textEdit object is destroyed
            // after endText() so we need to instead copy the information between an
            // old and new element that we can push and pop to recover.
            undo->addUndoAction(std::make_unique<TextBoxUndoAction>(page, layer, txt, this->oldtext));
        }
    }

    delete this->textEditor;
    this->textEditor = nullptr;
    this->rerenderPage();
}

void XojPageView::startText(double x, double y) {
    this->xournal->endTextAllPages(this);
    this->xournal->getControl()->getSearchBar()->showSearchBar(false);

    if (this->textEditor != nullptr) {
        Text* text = this->textEditor->getText();
        GdkRectangle matchRect = {gint(x - 10), gint(y - 10), 20, 20};
        if (!text->intersectsArea(&matchRect)) {
            endText();
        } else {
            this->textEditor->mousePressed(x - text->getX(), y - text->getY());
        }
    }

    if (this->textEditor == nullptr) {
        // Is there already a textfield?
        Text* text = nullptr;

        for (Element* e: *this->page->getSelectedLayer()->getElements()) {
            if (e->getType() == ELEMENT_TEXT) {
                GdkRectangle matchRect = {gint(x - 10), gint(y - 10), 20, 20};
                if (e->intersectsArea(&matchRect)) {
                    text = dynamic_cast<Text*>(e);
                    break;
                }
            }
        }

        bool ownText = false;
        if (text == nullptr) {
            ToolHandler* h = xournal->getControl()->getToolHandler();
            ownText = true;
            text = new Text();
            text->setColor(h->getColor());
            text->setFont(settings->getFont());
            text->setX(x);
            text->setY(y - text->getElementHeight() / 2);

            if (xournal->getControl()->getAudioController()->isRecording()) {
                string audioFilename = xournal->getControl()->getAudioController()->getAudioFilename();
                size_t sttime = xournal->getControl()->getAudioController()->getStartTime();
                size_t milliseconds = ((g_get_monotonic_time() / 1000) - sttime);
                text->setTimestamp(milliseconds);
                text->setAudioFilename(audioFilename);
            }
        } else {

            // We can try to add an undo action here. The initial text shows up in this
            // textEditor element.
            this->oldtext = text;
            // text = new Text(*oldtext);
            // need to clone the old text so that references still work properly.
            // cloning breaks things a little. do it manually
            text = new Text();
            text->setX(oldtext->getX());
            text->setY(oldtext->getY());
            text->setColor(oldtext->getColor());
            text->setFont(oldtext->getFont());
            text->setText(oldtext->getText());
            text->setTimestamp(oldtext->getTimestamp());
            text->setAudioFilename(oldtext->getAudioFilename());

            Layer* layer = this->page->getSelectedLayer();
            layer->removeElement(this->oldtext, false);
            layer->addElement(text);
            // perform the old swap onto the new text drawn.
        }

        this->textEditor = new TextEditor(this, xournal->getWidget(), text, ownText);
        if (!ownText) {
            this->textEditor->mousePressed(x - text->getX(), y - text->getY());
        }

        this->rerenderPage();
    }
}

auto XojPageView::onButtonPressEvent(const PositionInputData& pos) -> bool {
    Control* control = xournal->getControl();

    if (!this->selected) {
        control->firePageSelected(this->page);
    }

    ToolHandler* h = control->getToolHandler();

    double x = pos.x;
    double y = pos.y;

    if (x < 0 || y < 0) {
        return false;
    }

    double zoom = xournal->getZoom();
    x /= zoom;
    y /= zoom;

    XournalppCursor* cursor = xournal->getCursor();
    cursor->setMouseDown(true);

    if ((h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HILIGHTER) &&
                h->getDrawingType() != DRAWING_TYPE_SPLINE ||
        (h->getToolType() == TOOL_ERASER && h->getEraserType() == ERASER_TYPE_WHITEOUT)) {
        delete this->inputHandler;
        this->inputHandler = nullptr;

        if (h->getDrawingType() == DRAWING_TYPE_LINE) {
            this->inputHandler = new RulerHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_RECTANGLE) {
            this->inputHandler = new RectangleHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_CIRCLE) {
            this->inputHandler = new CircleHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_ARROW) {
            this->inputHandler = new ArrowHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM) {
            this->inputHandler = new CoordinateSystemHandler(this->xournal, this, getPage());
        } else {
            this->inputHandler = new StrokeHandler(this->xournal, this, getPage());
        }

        this->inputHandler->onButtonPressEvent(pos);
    } else if ((h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HILIGHTER) &&
               h->getDrawingType() == DRAWING_TYPE_SPLINE) {
        if (!this->inputHandler) {
            this->inputHandler = new SplineHandler(this->xournal, this, getPage());
        }
        this->inputHandler->onButtonPressEvent(pos);
    } else if (h->getToolType() == TOOL_ERASER) {
        this->eraser->erase(x, y);
        this->inEraser = true;
    } else if (h->getToolType() == TOOL_VERTICAL_SPACE) {
        this->verticalSpace = new VerticalToolHandler(this, this->page, this->settings, y, zoom);
    } else if (h->getToolType() == TOOL_SELECT_RECT || h->getToolType() == TOOL_SELECT_REGION ||
               h->getToolType() == TOOL_PLAY_OBJECT || h->getToolType() == TOOL_SELECT_OBJECT) {
        if (h->getToolType() == TOOL_SELECT_RECT) {
            if (this->selection) {
                delete this->selection;
                this->selection = nullptr;
                repaintPage();
            }
            this->selection = new RectSelection(x, y, this);
        } else if (h->getToolType() == TOOL_SELECT_REGION) {
            if (this->selection) {
                delete this->selection;
                this->selection = nullptr;
                repaintPage();
            }
            this->selection = new RegionSelect(x, y, this);
        } else if (h->getToolType() == TOOL_SELECT_OBJECT) {
            SelectObject select(this);
            select.at(x, y);
        } else if (h->getToolType() == TOOL_PLAY_OBJECT) {
            PlayObject play(this);
            play.at(x, y);
            if (play.playbackStatus) {
                auto& status = *play.playbackStatus;
                if (!status.success) {
                    string message = FS(_F("Unable to play audio recording {1}") % status.filename);
                    XojMsgBox::showErrorToUser(this->xournal->getControl()->getGtkWindow(), message);
                }
            }
        }
    } else if (h->getToolType() == TOOL_TEXT) {
        startText(x, y);
    } else if (h->getToolType() == TOOL_IMAGE) {
        ImageHandler imgHandler(control, this);
        imgHandler.insertImage(x, y);
    } else if (h->getToolType() == TOOL_FLOATING_TOOLBOX) {
        gint wx = 0, wy = 0;
        GtkWidget* widget = xournal->getWidget();
        gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);

        wx += std::lround(pos.x) + this->getX();
        wy += std::lround(pos.y) + this->getY();

        control->getWindow()->floatingToolbox->show(wx, wy);
    }

    return true;
}

auto XojPageView::onButtonDoublePressEvent(const PositionInputData& pos) -> bool {
    // This method assumes that it is called after onButtonPressEvent but before
    // onButtonReleaseEvent
    double zoom = this->xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;
    if (x < 0 || y < 0) {
        return false;
    }

    ToolHandler* toolHandler = this->xournal->getControl()->getToolHandler();
    ToolType toolType = toolHandler->getToolType();
    bool isSelectTool = toolType == TOOL_SELECT_OBJECT || TOOL_SELECT_RECT || TOOL_SELECT_REGION;

    DrawingType drawingType = toolHandler->getDrawingType();

    EditSelection* selection = xournal->getSelection();
    bool hasNoModifiers = !pos.isShiftDown() && !pos.isControlDown();

    if (hasNoModifiers && isSelectTool && selection != nullptr) {
        // Find a selected object under the cursor, if possible. The selection doesn't change the
        // element coordinates until it is finalized, so we need to use position relative to the
        // original coordinates of the selection.
        double origx = x - (selection->getXOnView() - selection->getOriginalXOnView());
        double origy = y - (selection->getYOnView() - selection->getOriginalYOnView());
        std::vector<Element*>* elems = selection->getElements();
        auto it = std::find_if(elems->begin(), elems->end(),
                               [&](Element*& elem) { return elem->intersectsArea(origx - 5, origy - 5, 5, 5); });
        if (it != elems->end()) {
            // Enter editing mode on the selected object
            Element* object = *it;
            ElementType elemType = object->getType();
            if (elemType == ELEMENT_TEXT) {
                this->xournal->clearSelection();
                toolHandler->selectTool(TOOL_TEXT);
                // Simulate a button press; there's too many things that we
                // could forget to do if we manually call startText
                this->onButtonPressEvent(pos);
            } else if (elemType == ELEMENT_TEXIMAGE) {
                Control* control = this->xournal->getControl();
                this->xournal->clearSelection();
                auto* sel = new EditSelection(control->getUndoRedoHandler(), object, this, this->getPage());
                this->xournal->setSelection(sel);
                control->runLatex();
            }
        }
    } else if (toolType == TOOL_TEXT) {
        this->startText(x, y);
        this->textEditor->selectAtCursor(TextEditor::SelectType::word);
    } else if (drawingType == DRAWING_TYPE_SPLINE) {
        if (this->inputHandler) {
            this->inputHandler->onButtonDoublePressEvent(pos);
            delete this->inputHandler;
            this->inputHandler = nullptr;
        }
    }

    return true;
}

auto XojPageView::onButtonTriplePressEvent(const PositionInputData& pos) -> bool {
    // This method assumes that it is called after onButtonDoubleEvent but before
    // onButtonReleaseEvent
    double zoom = this->xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;
    if (x < 0 || y < 0) {
        return false;
    }

    ToolHandler* toolHandler = this->xournal->getControl()->getToolHandler();

    if (toolHandler->getToolType() == TOOL_TEXT) {
        this->startText(x, y);
        this->textEditor->selectAtCursor(TextEditor::SelectType::paragraph);
    }
    return true;
}

void XojPageView::resetShapeRecognizer() {
    if (this->inputHandler != nullptr) {
        this->inputHandler->resetShapeRecognizer();
    }
}

auto XojPageView::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;

    ToolHandler* h = xournal->getControl()->getToolHandler();

    if (containsPoint(std::lround(x), std::lround(y), true) && this->inputHandler &&
        this->inputHandler->onMotionNotifyEvent(pos)) {
        // input handler used this event
    } else if (this->selection) {
        this->selection->currentPos(x, y);
    } else if (this->verticalSpace) {
        this->verticalSpace->currentPos(x, y);
    } else if (this->textEditor) {
        XournalppCursor* cursor = getXournal()->getCursor();
        cursor->setInvisible(false);

        Text* text = this->textEditor->getText();
        this->textEditor->mouseMoved(x - text->getX(), y - text->getY());
    } else if (h->getToolType() == TOOL_ERASER && h->getEraserType() != ERASER_TYPE_WHITEOUT && this->inEraser) {
        this->eraser->erase(x, y);
    }

    return false;
}

auto XojPageView::onButtonReleaseEvent(const PositionInputData& pos) -> bool {
    Control* control = xournal->getControl();

    if (this->inputHandler) {
        this->inputHandler->onButtonReleaseEvent(pos);

        if (this->inputHandler->userTapped) {
            bool doAction = control->getSettings()->getDoActionOnStrokeFiltered();
            if (control->getSettings()->getTrySelectOnStrokeFiltered()) {
                double zoom = xournal->getZoom();
                SelectObject select(this);
                if (select.at(pos.x / zoom, pos.y / zoom)) {
                    doAction = false;  // selection made.. no action.
                }
            }

            if (doAction)  // pop up a menu
            {
                gint wx = 0, wy = 0;
                GtkWidget* widget = xournal->getWidget();
                gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);
                wx += std::lround(pos.x + this->getX());
                wy += std::lround(pos.y + this->getY());
                control->getWindow()->floatingToolbox->show(wx, wy);
            }
        }

        ToolHandler* h = control->getToolHandler();
        bool isDrawingTypeSpline = h->getDrawingType() == DRAWING_TYPE_SPLINE;
        if (!isDrawingTypeSpline || !this->inputHandler->getStroke()) {  // The Spline Tool finalizes drawing manually
            delete this->inputHandler;
            this->inputHandler = nullptr;
        }
    }

    if (this->inEraser) {
        this->inEraser = false;
        Document* doc = this->xournal->getControl()->getDocument();
        doc->lock();
        this->eraser->finalize();
        doc->unlock();
    }

    if (this->verticalSpace) {
        control->getUndoRedoHandler()->addUndoAction(this->verticalSpace->finalize());
        delete this->verticalSpace;
        this->verticalSpace = nullptr;
    }

    if (this->selection) {
        if (this->selection->finalize(this->page)) {
            xournal->setSelection(new EditSelection(control->getUndoRedoHandler(), this->selection, this));
            delete this->selection;
            this->selection = nullptr;
        } else {
            double zoom = xournal->getZoom();
            if (this->selection->userTapped(zoom)) {
                SelectObject select(this);
                select.at(pos.x / zoom, pos.y / zoom);
            }
            delete this->selection;
            this->selection = nullptr;

            repaintPage();
        }
    } else if (this->textEditor) {
        this->textEditor->mouseReleased();
    }

    return false;
}

auto XojPageView::onKeyPressEvent(GdkEventKey* event) -> bool {
    // Esc leaves text edition
    if (event->keyval == GDK_KEY_Escape) {
        if (this->textEditor) {
            endText();
            return true;
        }
        if (xournal->getSelection()) {
            xournal->clearSelection();
            return true;
        }

        return false;
    }

    if (this->textEditor) {
        return this->textEditor->onKeyPressEvent(event);
    }


    if (this->inputHandler) {
        return this->inputHandler->onKeyEvent(event);
    }


    return false;
}

auto XojPageView::onKeyReleaseEvent(GdkEventKey* event) -> bool {
    if (this->textEditor && this->textEditor->onKeyReleaseEvent(event)) {
        return true;
    }

    if (this->inputHandler && this->inputHandler->onKeyEvent(event)) {
        DrawingType drawingType = this->xournal->getControl()->getToolHandler()->getDrawingType();
        if (drawingType == DRAWING_TYPE_SPLINE) {  // Spline drawing has been finalized
            if (this->inputHandler) {
                delete this->inputHandler;
                this->inputHandler = nullptr;
            }
        }

        return true;
    }

    return false;
}

void XojPageView::rerenderPage() {
    this->rerenderComplete = true;
    this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void XojPageView::repaintPage() { xournal->getRepaintHandler()->repaintPage(this); }

void XojPageView::repaintArea(double x1, double y1, double x2, double y2) {
    double zoom = xournal->getZoom();
    xournal->getRepaintHandler()->repaintPageArea(this, std::lround(x1 * zoom) - 10, std::lround(y1 * zoom) - 10,
                                                  std::lround(x2 * zoom) + 20, std::lround(y2 * zoom) + 20);
}

void XojPageView::rerenderRect(double x, double y, double width, double height) {
    int rx = std::lround(std::max(x - 10, 0.0));
    int ry = std::lround(std::max(y - 10, 0.0));
    int rwidth = std::lround(width + 20);
    int rheight = std::lround(height + 20);

    addRerenderRect(rx, ry, rwidth, rheight);
}

void XojPageView::addRerenderRect(double x, double y, double width, double height) {
    if (this->rerenderComplete) {
        return;
    }

    auto rect = Rectangle<double>{x, y, width, height};

    g_mutex_lock(&this->repaintRectMutex);

    for (auto&& r: this->rerenderRects) {
        // its faster to redraw only one rect than repaint twice the same area
        // so loop through the rectangles to be redrawn, if new rectangle
        // intersects any of them, replace it by the union with the new one
        if (r.intersects(rect)) {
            r.unite(rect);
            g_mutex_unlock(&this->repaintRectMutex);
            return;
        }
    }

    this->rerenderRects.push_back(rect);
    g_mutex_unlock(&this->repaintRectMutex);

    this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void XojPageView::setSelected(bool selected) {
    this->selected = selected;

    if (selected) {
        this->xournal->requestFocus();
        this->xournal->getRepaintHandler()->repaintPageBorder(this);
    }
}

auto XojPageView::cut() -> bool {
    if (this->textEditor) {
        this->textEditor->cutToClipboard();
        return true;
    }
    return false;
}

auto XojPageView::copy() -> bool {
    if (this->textEditor) {
        this->textEditor->copyToCliboard();
        return true;
    }
    return false;
}

auto XojPageView::paste() -> bool {
    if (this->textEditor) {
        this->textEditor->pasteFromClipboard();
        return true;
    }
    return false;
}

auto XojPageView::actionDelete() -> bool {
    if (this->textEditor) {
        this->textEditor->deleteFromCursor(GTK_DELETE_CHARS, 1);
        return true;
    }
    return false;
}

void XojPageView::drawLoadingPage(cairo_t* cr) {
    static const string txtLoading = _("Loading...");

    double zoom = xournal->getZoom();
    int dispWidth = getDisplayWidth();
    int dispHeight = getDisplayHeight();

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, dispWidth, dispHeight);
    cairo_fill(cr);

    cairo_scale(cr, zoom, zoom);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 32.0);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, txtLoading.c_str(), &ex);
    cairo_move_to(cr, (page->getWidth() - ex.width) / 2 - ex.x_bearing,
                  (page->getHeight() - ex.height) / 2 - ex.y_bearing);
    cairo_show_text(cr, txtLoading.c_str());

    rerenderPage();
}

/**
 * Does the painting, called in synchronized block
 */
void XojPageView::paintPageSync(cairo_t* cr, GdkRectangle* rect) {
    if (this->crBuffer == nullptr) {
        drawLoadingPage(cr);
        return;
    }

    double zoom = xournal->getZoom();
    int dispWidth = getDisplayWidth();

    cairo_save(cr);

    double width = cairo_image_surface_get_width(this->crBuffer);

    bool rerender = true;
    if (width / xournal->getDpiScaleFactor() == dispWidth) {
        rerender = false;
    }

    if (width != dispWidth) {
        double scale = (static_cast<double>(dispWidth)) / (width);

        // Scale current image to fit the zoom level
        cairo_scale(cr, scale, scale);
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);

        cairo_set_source_surface(cr, this->crBuffer, 0, 0);

        if (rerender) {
            rerenderPage();
        }

        rect = nullptr;
    } else {
        cairo_set_source_surface(cr, this->crBuffer, 0, 0);
    }

    if (rect) {
        cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
        cairo_fill(cr);

#ifdef DEBUG_SHOW_PAINT_BOUNDS
        cairo_set_source_rgb(cr, 1.0, 0.5, 1.0);
        cairo_set_line_width(cr, 1. / zoom);
        cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
        cairo_stroke(cr);
#endif
    } else {
        cairo_paint(cr);
    }

    cairo_restore(cr);

    // don't paint this with scale, because it needs a 1:1 zoom
    if (this->verticalSpace) {
        this->verticalSpace->paint(cr, rect, zoom);
    }

    if (this->textEditor) {
        cairo_scale(cr, zoom, zoom);
        this->textEditor->paint(cr, rect, zoom);
    }
    if (this->selection) {
        cairo_scale(cr, zoom, zoom);
        this->selection->paint(cr, rect, zoom);
    }

    if (this->search) {
        cairo_save(cr);
        cairo_scale(cr, zoom, zoom);
        this->search->paint(cr, rect, zoom, getSelectionColor());
        cairo_restore(cr);
    }

    if (this->inputHandler) {
        int dpiScaleFactor = xournal->getDpiScaleFactor();
        cairo_scale(cr, 1.0 / dpiScaleFactor, 1.0 / dpiScaleFactor);
        this->inputHandler->draw(cr);
    }
}

auto XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect) -> bool {
    g_mutex_lock(&this->drawingMutex);

    paintPageSync(cr, rect);

    g_mutex_unlock(&this->drawingMutex);
    return true;
}

auto XojPageView::containsY(int y) const -> bool {
    return (y >= this->getY() && y <= (this->getY() + this->getDisplayHeight()));
}

/**
 * GETTER / SETTER
 */

auto XojPageView::isSelected() const -> bool { return selected; }

auto XojPageView::getBufferPixels() -> int {
    if (crBuffer) {
        return cairo_image_surface_get_width(crBuffer) * cairo_image_surface_get_height(crBuffer);
    }
    return 0;
}

auto XojPageView::getSelectionColor() -> GdkRGBA { return Util::rgb_to_GdkRGBA(settings->getSelectionColor()); }

auto XojPageView::getTextEditor() -> TextEditor* { return textEditor; }

auto XojPageView::getX() const -> int { return this->dispX; }

void XojPageView::setX(int x) { this->dispX = x; }

auto XojPageView::getY() const -> int { return this->dispY; }

void XojPageView::setY(int y) { this->dispY = y; }

void XojPageView::setMappedRowCol(int row, int col) {
    this->mappedRow = row;
    this->mappedCol = col;
}


auto XojPageView::getMappedRow() const -> int { return this->mappedRow; }


auto XojPageView::getMappedCol() const -> int { return this->mappedCol; }


auto XojPageView::getPage() -> PageRef { return page; }

auto XojPageView::getXournal() -> XournalView* { return this->xournal; }

auto XojPageView::getHeight() const -> double { return this->page->getHeight(); }

auto XojPageView::getWidth() const -> double { return this->page->getWidth(); }

auto XojPageView::getDisplayWidth() const -> int {
    return std::lround(this->page->getWidth() * this->xournal->getZoom());
}

auto XojPageView::getDisplayHeight() const -> int {
    return std::lround(this->page->getHeight() * this->xournal->getZoom());
}

auto XojPageView::getSelectedTex() -> TexImage* {
    EditSelection* theSelection = this->xournal->getSelection();
    if (!theSelection) {
        return nullptr;
    }

    for (Element* e: *theSelection->getElements()) {
        if (e->getType() == ELEMENT_TEXIMAGE) {
            return dynamic_cast<TexImage*>(e);
        }
    }
    return nullptr;
}

auto XojPageView::getSelectedText() -> Text* {
    EditSelection* theSelection = this->xournal->getSelection();
    if (!theSelection) {
        return nullptr;
    }

    for (Element* e: *theSelection->getElements()) {
        if (e->getType() == ELEMENT_TEXT) {
            return dynamic_cast<Text*>(e);
        }
    }
    return nullptr;
}

auto XojPageView::getRect() const -> Rectangle<double> {
    return Rectangle<double>(getX(), getY(), getDisplayWidth(), getDisplayHeight());
}

void XojPageView::rectChanged(Rectangle<double>& rect) { rerenderRect(rect.x, rect.y, rect.width, rect.height); }

void XojPageView::rangeChanged(Range& range) { rerenderRange(range); }

void XojPageView::pageChanged() { rerenderPage(); }

void XojPageView::elementChanged(Element* elem) {
    if (this->inputHandler && elem == this->inputHandler->getStroke()) {
        g_mutex_lock(&this->drawingMutex);

        cairo_t* cr = cairo_create(this->crBuffer);

        this->inputHandler->draw(cr);

        cairo_destroy(cr);

        g_mutex_unlock(&this->drawingMutex);
    } else {
        rerenderElement(elem);
    }
}
