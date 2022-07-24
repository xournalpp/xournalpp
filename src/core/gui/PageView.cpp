#include "PageView.h"

#include <algorithm>  // for max, find_if
#include <cmath>      // for lround
#include <cstdlib>    // for size_t
#include <memory>     // for __shared_ptr_access
#include <optional>   // for optional
#include <utility>    // for move

#include <gdk/gdk.h>         // for GdkRectangle, Gdk...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Escape
#include <glib.h>            // for gint, g_get_curre...
#include <gtk/gtk.h>         // for gtk_widget_get_to...

#include "control/AudioController.h"                // for AudioController
#include "control/Control.h"                        // for Control
#include "control/SearchControl.h"                  // for SearchControl
#include "control/ToolEnums.h"                      // for DRAWING_TYPE_SPLINE
#include "control/ToolHandler.h"                    // for ToolHandler
#include "control/jobs/XournalScheduler.h"          // for XournalScheduler
#include "control/settings/Settings.h"              // for Settings
#include "control/tools/ArrowHandler.h"             // for ArrowHandler
#include "control/tools/CoordinateSystemHandler.h"  // for CoordinateSystemH...
#include "control/tools/EditSelection.h"            // for EditSelection
#include "control/tools/EllipseHandler.h"           // for EllipseHandler
#include "control/tools/EraseHandler.h"             // for EraseHandler
#include "control/tools/ImageHandler.h"             // for ImageHandler
#include "control/tools/InputHandler.h"             // for InputHandler
#include "control/tools/PdfElemSelection.h"         // for PdfElemSelection
#include "control/tools/RectangleHandler.h"         // for RectangleHandler
#include "control/tools/RulerHandler.h"             // for RulerHandler
#include "control/tools/Selection.h"                // for Selection, RectSe...
#include "control/tools/SplineHandler.h"            // for SplineHandler
#include "control/tools/StrokeHandler.h"            // for StrokeHandler
#include "control/tools/VerticalToolHandler.h"      // for VerticalToolHandler
#include "control/zoom/ZoomControl.h"               // for ZoomControl
#include "gui/FloatingToolbox.h"                    // for FloatingToolbox
#include "gui/MainWindow.h"                         // for MainWindow
#include "gui/PdfFloatingToolbox.h"                 // for PdfFloatingToolbox
#include "gui/SearchBar.h"                          // for SearchBar
#include "gui/inputdevices/PositionInputData.h"     // for PositionInputData
#include "model/Document.h"                         // for Document
#include "model/Element.h"                          // for Element, ELEMENT_...
#include "model/Layer.h"                            // for Layer
#include "model/PageRef.h"                          // for PageRef
#include "model/Stroke.h"                           // for Stroke
#include "model/TexImage.h"                         // for TexImage
#include "model/Text.h"                             // for Text
#include "model/XojPage.h"                          // for XojPage
#include "pdf/base/XojPdfPage.h"                    // for XojPdfPageSPtr
#include "undo/DeleteUndoAction.h"                  // for DeleteUndoAction
#include "undo/InsertUndoAction.h"                  // for InsertUndoAction
#include "undo/MoveUndoAction.h"                    // for MoveUndoAction
#include "undo/TextBoxUndoAction.h"                 // for TextBoxUndoAction
#include "undo/UndoRedoHandler.h"                   // for UndoRedoHandler
#include "util/Color.h"                             // for rgb_to_GdkRGBA
#include "util/Rectangle.h"                         // for Rectangle
#include "util/Util.h"                              // for npos
#include "util/XojMsgBox.h"                         // for XojMsgBox
#include "util/i18n.h"                              // for FS, _, _F
#include "view/DebugShowRepaintBounds.h"            // for IF_DEBUG_REPAINT

#include "PageViewFindObjectHelper.h"  // for SelectObject, Pla...
#include "RepaintHandler.h"            // for RepaintHandler
#include "TextEditor.h"                // for TextEditor, TextE...
#include "XournalView.h"               // for XournalView
#include "XournalppCursor.h"           // for XournalppCursor
#include "config-debug.h"              // for DEBUG_SHOW_PAINT_...
#include "filesystem.h"                // for path

class Range;

using std::string;
using xoj::util::Rectangle;

XojPageView::XojPageView(XournalView* xournal, const PageRef& page):
        page(page),
        xournal(xournal),
        settings(xournal->getControl()->getSettings()),
        eraser(new EraseHandler(xournal->getControl()->getUndoRedoHandler(), xournal->getControl()->getDocument(),
                                this->page, xournal->getControl()->getToolHandler(), this)),
        oldtext(nullptr) {
    this->registerToHandler(this->page);
}

XojPageView::~XojPageView() {
    this->unregisterFromHandler();

    this->xournal->getControl()->getScheduler()->removePage(this);
    delete this->inputHandler;
    delete this->eraser;
    endText();
    deleteViewBuffer();  // Ensures the mutex is locked during the buffer's destruction
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
    if (!this->crBuffer) {
        return -1;
    }

    return this->lastVisibleTime;
}

void XojPageView::deleteViewBuffer() {
    std::lock_guard lock(this->drawingMutex);
    this->crBuffer.reset();
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

        auto pNr = this->page->getPdfPageNr();
        XojPdfPageSPtr pdf = nullptr;
        if (pNr != npos) {
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
        auto pos = layer->indexOf(txt);
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
    this->xournal->getControl()->getWindow()->setFontButtonFont(settings->getFont());
    this->rerenderPage();
}

void XojPageView::startText(double x, double y) {
    this->xournal->endTextAllPages(this);
    this->xournal->getControl()->getSearchBar()->showSearchBar(false);

    if (this->textEditor != nullptr) {
        Text* text = this->textEditor->getText();
        GdkRectangle matchRect = {gint(x), gint(y), 1, 1};
        if (!text->intersectsArea(&matchRect)) {
            endText();
        } else {
            this->textEditor->mousePressed(x - text->getX(), y - text->getY());
        }
    }

    if (this->textEditor == nullptr) {
        // Is there already a textfield?
        Text* text = nullptr;

        for (Element* e: this->page->getSelectedLayer()->getElements()) {
            if (e->getType() == ELEMENT_TEXT) {
                GdkRectangle matchRect = {gint(x), gint(y), 1, 1};
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
                fs::path audioFilename = xournal->getControl()->getAudioController()->getAudioFilename();
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
            this->xournal->getControl()->getWindow()->setFontButtonFont(oldtext->getFont());
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

    if (((h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HIGHLIGHTER) &&
         h->getDrawingType() != DRAWING_TYPE_SPLINE) ||
        (h->getToolType() == TOOL_ERASER && h->getEraserType() == ERASER_TYPE_WHITEOUT)) {
        delete this->inputHandler;
        this->inputHandler = nullptr;

        if (h->getDrawingType() == DRAWING_TYPE_LINE) {
            this->inputHandler = new RulerHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_RECTANGLE) {
            this->inputHandler = new RectangleHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_ELLIPSE) {
            this->inputHandler = new EllipseHandler(this->xournal, this, getPage());
        } else if (h->getDrawingType() == DRAWING_TYPE_ARROW) {
            this->inputHandler = new ArrowHandler(this->xournal, this, getPage(), false);
        } else if (h->getDrawingType() == DRAWING_TYPE_DOUBLE_ARROW) {
            this->inputHandler = new ArrowHandler(this->xournal, this, getPage(), true);
        } else if (h->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM) {
            this->inputHandler = new CoordinateSystemHandler(this->xournal, this, getPage());
        } else {
            this->inputHandler = new StrokeHandler(this->xournal, this, getPage());
        }

        this->inputHandler->onButtonPressEvent(pos);
    } else if ((h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HIGHLIGHTER) &&
               h->getDrawingType() == DRAWING_TYPE_SPLINE) {
        if (!this->inputHandler) {
            this->inputHandler = new SplineHandler(this->xournal, this, getPage());
        }
        this->inputHandler->onButtonPressEvent(pos);
    } else if (h->getToolType() == TOOL_ERASER) {
        this->eraser->erase(x, y);
        this->inEraser = true;
    } else if (h->getToolType() == TOOL_VERTICAL_SPACE) {
        if (this->verticalSpace) {
            control->getUndoRedoHandler()->addUndoAction(this->verticalSpace->finalize());
            this->getXournal()->getControl()->getZoomControl()->removeZoomListener(this->verticalSpace);
            delete this->verticalSpace;
        }
        auto* window = gtk_widget_get_window(xournal->getWidget());
        auto* zoomControl = this->getXournal()->getControl()->getZoomControl();
        this->verticalSpace =
                new VerticalToolHandler(this, this->page, this->settings, y, pos.isControlDown(), zoomControl, window);
        zoomControl->addZoomListener(this->verticalSpace);
    } else if (h->getToolType() == TOOL_SELECT_RECT || h->getToolType() == TOOL_SELECT_REGION ||
               h->getToolType() == TOOL_PLAY_OBJECT || h->getToolType() == TOOL_SELECT_OBJECT ||
               h->getToolType() == TOOL_SELECT_PDF_TEXT_LINEAR || h->getToolType() == TOOL_SELECT_PDF_TEXT_RECT) {
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
        } else if (h->getToolType() == TOOL_SELECT_PDF_TEXT_LINEAR || h->getToolType() == TOOL_SELECT_PDF_TEXT_RECT) {
            // so if we selected something && the pdf selection toolbox is hidden && we hit within the selection
            // we could call the pdf floating toolbox again
            auto* pdfToolbox = control->getWindow()->getPdfToolbox();

            if (pdfToolbox->hasSelection()) {
                bool isPdfToolboxHidden = pdfToolbox->isHidden();
                bool keepOldSelection = isPdfToolboxHidden && pdfToolbox->getSelection()->contains(x, y);
                if (!keepOldSelection) {
                    pdfToolbox->userCancelSelection();
                    repaintPage();
                } else {
                    showPdfToolbox(pos);
                }
            }

            if (this->page->getPdfPageNr() != npos && !pdfToolbox->hasSelection()) {
                pdfToolbox->selectionStyle = PdfElemSelection::selectionStyleForToolType(h->getToolType());
                pdfToolbox->newSelection(x, y, this);
            }
        } else if (h->getToolType() == TOOL_SELECT_OBJECT) {
            SelectObject select(this);
            select.at(x, y);
        } else if (h->getToolType() == TOOL_PLAY_OBJECT) {
            PlayObject play(this);
            play.at(x, y);
            if (play.playbackStatus) {
                auto& status = *play.playbackStatus;
                if (!status.success) {
                    string message = FS(_F("Unable to play audio recording {1}") % status.filename.u8string());
                    XojMsgBox::showErrorToUser(this->xournal->getControl()->getGtkWindow(), message);
                }
            }
        }
    } else if (h->getToolType() == TOOL_TEXT) {
        startText(x, y);
    } else if (h->getToolType() == TOOL_IMAGE) {
        ImageHandler imgHandler(control, this);
        imgHandler.insertImage(x, y);
    }

    this->onButtonClickEvent(pos);

    return true;
}

auto XojPageView::onButtonClickEvent(const PositionInputData& pos) -> bool {
    Control* control = xournal->getControl();
    double x = pos.x;
    double y = pos.y;

    if (x < 0 || y < 0) {
        return false;
    }

    ToolHandler* h = control->getToolHandler();

    if (h->getToolType() == TOOL_FLOATING_TOOLBOX) {
        this->showFloatingToolbox(pos);
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
    DrawingType drawingType = toolHandler->getDrawingType();

    EditSelection* selection = xournal->getSelection();
    bool hasNoModifiers = !pos.isShiftDown() && !pos.isControlDown();

    if (hasNoModifiers && selection != nullptr) {
        // Find a selected object under the cursor, if possible. The selection doesn't change the
        // element coordinates until it is finalized, so we need to use position relative to the
        // original coordinates of the selection.
        double origx = x - (selection->getXOnView() - selection->getOriginalXOnView());
        double origy = y - (selection->getYOnView() - selection->getOriginalYOnView());
        const std::vector<Element*>& elems = selection->getElements();
        auto it = std::find_if(elems.begin(), elems.end(),
                               [&](Element* elem) { return elem->intersectsArea(origx - 5, origy - 5, 5, 5); });
        if (it != elems.end()) {
            // Enter editing mode on the selected object
            Element* object = *it;
            ElementType elemType = object->getType();
            if (elemType == ELEMENT_TEXT) {
                this->xournal->clearSelection();
                toolHandler->selectTool(TOOL_TEXT);
                toolHandler->fireToolChanged();
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
    } else if (toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT) {
        auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
        if (auto* selection = pdfToolbox->getSelection()) {
            pdfToolbox->selectionStyle = XojPdfPageSelectionStyle::Word;
            selection->currentPos(x, y, pdfToolbox->selectionStyle);
        }
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
    ToolType toolType = toolHandler->getToolType();

    if (toolType == TOOL_TEXT) {
        this->startText(x, y);
        this->textEditor->selectAtCursor(TextEditor::SelectType::paragraph);
    } else if (toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT) {
        auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
        if (auto* selection = pdfToolbox->getSelection()) {
            pdfToolbox->selectionStyle = XojPdfPageSelectionStyle::Line;
            selection->currentPos(x, y, pdfToolbox->selectionStyle);
        }
    }
    return true;
}

auto XojPageView::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;

    ToolHandler* h = xournal->getControl()->getToolHandler();
    auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();

    if (containsPoint(std::lround(x), std::lround(y), true) && this->inputHandler &&
        this->inputHandler->onMotionNotifyEvent(pos)) {
        // input handler used this event
    } else if (this->selection) {
        this->selection->currentPos(x, y);
    } else if (auto* selection = pdfToolbox->getSelection(); selection && !selection->isFinalized()) {
        selection->currentPos(x, y, pdfToolbox->selectionStyle);
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

void XojPageView::onMotionCancelEvent() {
    if (this->inputHandler) {
        this->inputHandler->onMotionCancelEvent();
    }
}

auto XojPageView::showPdfToolbox(const PositionInputData& pos) -> void {
    gint wx = 0, wy = 0;
    GtkWidget* widget = xournal->getWidget();
    gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);

    wx += static_cast<gint>(std::lround(pos.x)) + this->getX();
    wy += static_cast<gint>(std::lround(pos.y)) + this->getY();

    auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
    pdfToolbox->show(wx, wy);
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
                this->showFloatingToolbox(pos);
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
        this->getXournal()->getControl()->getZoomControl()->removeZoomListener(this->verticalSpace);
        delete this->verticalSpace;
        this->verticalSpace = nullptr;
    }

    auto* pdfToolbox = control->getWindow()->getPdfToolbox();
    if (auto* selection = pdfToolbox->getSelection()) {
        if (!selection->isFinalized() && selection->finalizeSelectionAndRepaint(pdfToolbox->selectionStyle)) {
            // Selection was created, so reposition the toolbox and display.
            showPdfToolbox(pos);
        }
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
    if (this->textEditor) {
        if (this->textEditor->onKeyPressEvent(event)) {
            return true;
        }
    } else if (this->inputHandler) {
        if (this->inputHandler->onKeyEvent(event)) {
            return true;
        }
    } else if (this->verticalSpace) {
        if (this->verticalSpace->onKeyPressEvent(event)) {
            return true;
        }
    }

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

    if (this->verticalSpace && this->verticalSpace->onKeyReleaseEvent(event)) {
        return true;
    }

    return false;
}

void XojPageView::rerenderPage() {
    this->rerenderComplete = true;
    this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void XojPageView::repaintPage() const { xournal->getRepaintHandler()->repaintPage(this); }

void XojPageView::repaintArea(double x1, double y1, double x2, double y2) const {
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

    this->repaintRectMutex.lock();

    for (auto&& r: this->rerenderRects) {
        // its faster to redraw only one rect than repaint twice the same area
        // so loop through the rectangles to be redrawn, if new rectangle
        // intersects any of them, replace it by the union with the new one
        if (r.intersects(rect)) {
            r.unite(rect);
            this->repaintRectMutex.unlock();
            return;
        }
    }

    this->rerenderRects.push_back(rect);
    this->repaintRectMutex.unlock();

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

auto XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect) -> bool {

    double zoom = xournal->getZoom();
    {
        std::lock_guard lock(this->drawingMutex);  // Lock the mutex first
        xoj::util::CairoSaveGuard saveGuard(cr);   // see comment at the end of the scope
        if (!this->crBuffer) {
            drawLoadingPage(cr);
            return true;
        }

        int dispWidth = getDisplayWidth();
        cairo_scale(cr, zoom, zoom);

        double width = cairo_image_surface_get_width(this->crBuffer.get());

        if (width / xournal->getDpiScaleFactor() != dispWidth) {
            rerenderPage();
        }
        cairo_set_source_surface(cr, this->crBuffer.get(), 0, 0);

        if (rect) {
            cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
            cairo_fill(cr);
            IF_DEBUG_REPAINT({
                cairo_set_source_rgb(cr, 1.0, 0.5, 1.0);
                cairo_set_line_width(cr, 1. / zoom);
                cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
                cairo_stroke(cr);
            })
        } else {
            cairo_paint(cr);
        }
    }  // Restore the state of cr and then release the mutex
       // restoring the state of cr ensures this->crBuffer is not longer referenced as the source in cr.

    /**
     * All the tool painters below follow the assumption:
     *  * The given cairo context is in page coordinates: no further scaling/offset is ever required.
     *
     * To anyone adding another painter here: please keep this assumption true
     */
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_scale(cr, zoom, zoom);

    if (this->verticalSpace) {
        this->verticalSpace->paint(cr);
    }

    if (this->textEditor) {
        this->textEditor->paint(cr, zoom);
    }

    if (this->selection) {
        this->selection->paint(cr, zoom);
    }

    auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
    if (auto* selection = pdfToolbox->getSelection(); selection) {
        selection->paint(cr, pdfToolbox->selectionStyle);
    }

    if (this->search) {
        this->search->paint(cr, zoom, getSelectionColor());
    }

    if (this->inputHandler) {
        this->inputHandler->draw(cr);
    }
    return true;
}

/**
 * GETTER / SETTER
 */

auto XojPageView::isSelected() const -> bool { return selected; }

auto XojPageView::getBufferPixels() -> int {
    if (crBuffer) {
        return cairo_image_surface_get_width(crBuffer.get()) * cairo_image_surface_get_height(crBuffer.get());
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


auto XojPageView::getPage() const -> const PageRef { return page; }

auto XojPageView::getXournal() const -> XournalView* { return this->xournal; }

auto XojPageView::getHeight() const -> double { return this->page->getHeight(); }

auto XojPageView::getWidth() const -> double { return this->page->getWidth(); }

auto XojPageView::getDisplayWidth() const -> int {
    return std::lround(this->page->getWidth() * this->xournal->getZoom());
}

auto XojPageView::getDisplayHeight() const -> int {
    return std::lround(this->page->getHeight() * this->xournal->getZoom());
}

auto XojPageView::getDisplayWidthDouble() const -> double { return this->page->getWidth() * this->xournal->getZoom(); }

auto XojPageView::getDisplayHeightDouble() const -> double {
    return this->page->getHeight() * this->xournal->getZoom();
}

auto XojPageView::getSelectedTex() -> TexImage* {
    EditSelection* theSelection = this->xournal->getSelection();
    if (!theSelection) {
        return nullptr;
    }

    for (Element* e: theSelection->getElements()) {
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

    for (Element* e: theSelection->getElements()) {
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
        std::lock_guard lock(this->drawingMutex);
        xoj::util::CairoSPtr cr(cairo_create(this->crBuffer.get()));
        this->inputHandler->draw(cr.get());
    } else {
        rerenderElement(elem);
    }
}

void XojPageView::showFloatingToolbox(const PositionInputData& pos) {
    Control* control = xournal->getControl();

    gint wx = 0, wy = 0;
    GtkWidget* widget = xournal->getWidget();
    gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);

    wx += std::lround(pos.x) + this->getX();
    wy += std::lround(pos.y) + this->getY();

    control->getWindow()->floatingToolbox->show(wx, wy);
}
