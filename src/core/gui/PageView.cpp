#include "PageView.h"

#include <algorithm>  // for max, find_if
#include <cinttypes>  // for int64_t
#include <cstdint>    // for int64_t
#include <cstdlib>    // for size_t
#include <iomanip>    // for operator<<, quoted
#include <memory>     // for unique_ptr, make_...
#include <optional>   // for optional
#include <sstream>    // for operator<<, basic...
#include <tuple>      // for tuple, tie
#include <utility>    // for move

#include <gdk/gdk.h>         // for GdkRectangle, Gdk...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Escape
#include <glib-object.h>     // for G_CALLBACK, g_sig...
#include <glib.h>            // for gint, g_free, g_g...
#include <gtk/gtk.h>         // for GtkWidget, gtk_co...

#include "control/AudioController.h"                // for AudioController
#include "control/Control.h"                        // for Control
#include "control/ScrollHandler.h"                  // for ScrollHandler
#include "control/SearchControl.h"                  // for SearchControl
#include "control/Tool.h"                           // for Tool
#include "control/ToolEnums.h"                      // for DRAWING_TYPE_SPLINE
#include "control/ToolHandler.h"                    // for ToolHandler
#include "control/jobs/XournalScheduler.h"          // for XournalScheduler
#include "control/layer/LayerController.h"          // for LayerControl
#include "control/settings/Settings.h"              // for Settings
#include "control/tools/ArrowHandler.h"             // for ArrowHandler
#include "control/tools/CoordinateSystemHandler.h"  // for CoordinateSystemH...
#include "control/tools/EditSelection.h"            // for EditSelection
#include "control/tools/EllipseHandler.h"           // for EllipseHandler
#include "control/tools/EraseHandler.h"             // for EraseHandler
#include "control/tools/ImageHandler.h"             // for ImageHandler
#include "control/tools/ImageSizeSelection.h"       // for ImageSizeSelection
#include "control/tools/InputHandler.h"             // for InputHandler
#include "control/tools/PdfElemSelection.h"         // for PdfElemSelection
#include "control/tools/RectangleHandler.h"         // for RectangleHandler
#include "control/tools/RulerHandler.h"             // for RulerHandler
#include "control/tools/Selection.h"                // for RectSelection
#include "control/tools/SplineHandler.h"            // for SplineHandler
#include "control/tools/StrokeHandler.h"            // for StrokeHandler
#include "control/tools/TextEditor.h"               // for TextEditor, TextE...
#include "control/tools/VerticalToolHandler.h"      // for VerticalToolHandler
#include "gui/FloatingToolbox.h"                    // for FloatingToolbox
#include "gui/MainWindow.h"                         // for MainWindow
#include "gui/PdfFloatingToolbox.h"                 // for PdfFloatingToolbox
#include "gui/SearchBar.h"                          // for SearchBar
#include "gui/inputdevices/PositionInputData.h"     // for PositionInputData
#include "model/Document.h"                         // for Document
#include "model/Element.h"                          // for Element, ELEMENT_...
#include "model/Layer.h"                            // for Layer, Layer::Index
#include "model/LinkDestination.h"                  // for LinkDestination
#include "model/PageRef.h"                          // for PageRef
#include "model/Stroke.h"                           // for Stroke
#include "model/TexImage.h"                         // for TexImage
#include "model/Text.h"                             // for Text
#include "model/XojPage.h"                          // for XojPage
#include "pdf/base/XojPdfAction.h"                  // for XojPdfAction
#include "pdf/base/XojPdfDocument.h"                // for XojPdfDocument
#include "pdf/base/XojPdfPage.h"                    // for XojPdfRectangle
#include "undo/DeleteUndoAction.h"                  // for DeleteUndoAction
#include "undo/InsertUndoAction.h"                  // for InsertUndoAction
#include "undo/MoveUndoAction.h"                    // for MoveUndoAction
#include "undo/TextBoxUndoAction.h"                 // for TextBoxUndoAction
#include "undo/UndoRedoHandler.h"                   // for UndoRedoHandler
#include "util/Assert.h"                            // for xoj_assert
#include "util/Color.h"                             // for rgb_to_GdkRGBA
#include "util/Range.h"                             // for Range
#include "util/Rectangle.h"                         // for Rectangle
#include "util/Util.h"                              // for npos
#include "util/XojMsgBox.h"                         // for XojMsgBox
#include "util/gtk4_helper.h"                       // for gtk_box_append
#include "util/i18n.h"                              // for _F, FC, FS, _
#include "util/raii/CLibrariesSPtr.h"               // for adopt
#include "util/safe_casts.h"                        // for ceil_cast, floor_cast, round_cast
#include "util/serdesstream.h"                      // for serdes_stream
#include "view/DebugShowRepaintBounds.h"            // for IF_DEBUG_REPAINT
#include "view/overlays/OverlayView.h"              // for OverlayView, Tool...
#include "view/overlays/PdfElementSelectionView.h"  // for PdfElementSelecti...
#include "view/overlays/SearchResultView.h"         // for SearchResultView
#include "view/overlays/SelectionView.h"            // for SelectionView
#include "view/overlays/TextEditionView.h"          // for TextEditionView

#include "PageViewFindObjectHelper.h"  // for SelectObject, Pla...
#include "RepaintHandler.h"            // for RepaintHandler
#include "XournalView.h"               // for XournalView
#include "XournalppCursor.h"           // for XournalppCursor
#include "filesystem.h"                // for path

class OverlayBase;

using std::string;
using xoj::util::Rectangle;

XojPageView::XojPageView(XournalView* xournal, const PageRef& page):
        page(page),
        xournal(xournal),
        settings(xournal->getControl()->getSettings()),
        eraser(std::make_unique<EraseHandler>(xournal->getControl()->getUndoRedoHandler(),
                                              xournal->getControl()->getDocument(), this->page,
                                              xournal->getControl()->getToolHandler(), this)),
        oldtext(nullptr) {
    this->registerToHandler(this->page);
}

XojPageView::~XojPageView() {
    this->unregisterFromHandler();

    this->xournal->getControl()->getScheduler()->removePage(this);

    this->overlayViews.clear();
    endText();
    deleteViewBuffer();  // Ensures the mutex is locked during the buffer's destruction
}

void XojPageView::addOverlayView(std::unique_ptr<xoj::view::OverlayView> overlay) {
    this->overlayViews.emplace_back(std::move(overlay));
}

void XojPageView::setIsVisible(bool visible) { this->visible = visible; }

void XojPageView::deleteViewBuffer() {
    std::lock_guard lock(this->drawingMutex);
    this->buffer.reset();
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

auto XojPageView::searchTextOnPage(const std::string& text, size_t index, size_t* occurrences,
                                   XojPdfRectangle* matchRect) -> bool {
    if (!this->search) {
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
        this->search = std::make_unique<SearchControl>(page, pdf);
        this->overlayViews.emplace_back(std::make_unique<xoj::view::SearchResultView>(
                this->search.get(), this, settings->getSelectionColor(), settings->getActiveSelectionColor()));
    }

    bool found = this->search->search(text, index, occurrences, matchRect);

    repaintPage();

    return found;
}

void XojPageView::endText() { this->textEditor.reset(); }

void XojPageView::startText(double x, double y) {
    this->xournal->endTextAllPages(this);
    this->xournal->getControl()->getSearchBar()->showSearchBar(false);

    if (this->textEditor != nullptr) {
        const Text* text = this->textEditor->getTextElement();
        GdkRectangle matchRect = {gint(x), gint(y), 1, 1};
        if (!text->intersectsArea(&matchRect)) {
            endText();
        } else {
            this->textEditor->mousePressed(x - text->getX(), y - text->getY());
        }
    }

    if (this->textEditor == nullptr) {
        this->textEditor = std::make_unique<TextEditor>(xournal->getControl(), page, xournal->getWidget(), x, y);
        this->overlayViews.emplace_back(std::make_unique<xoj::view::TextEditionView>(this->textEditor.get(), this));
    }
}

#ifndef NDEBUG
// used in xoj_assert()
[[maybe_unused]] static bool hasNoViewOf(const std::vector<std::unique_ptr<xoj::view::OverlayView>>& views,
                                         const OverlayBase* o) {
    return std::find_if(views.begin(), views.end(), [o](auto& v) { return v->isViewOf(o); }) == views.end();
}
#endif

static void eraseViewsOf(std::vector<std::unique_ptr<xoj::view::OverlayView>>& views, const OverlayBase* o) {
    views.erase(std::remove_if(views.begin(), views.end(), [o](auto& v) { return v->isViewOf(o); }), views.end());
    xoj_assert(hasNoViewOf(views, o));
}

void XojPageView::endSpline() {
    if (SplineHandler* h = dynamic_cast<SplineHandler*>(this->inputHandler.get()); h) {
        h->finalizeSpline();
        xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
        this->inputHandler.reset();
    }
}

auto XojPageView::onButtonPressEvent(const PositionInputData& pos) -> bool {
    if (currentSequenceDeviceId) {
        // An input sequence is already under way from another device
        return false;
    }
    currentSequenceDeviceId = pos.deviceId;

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

        if (this->inputHandler) {
            /**
             * Due to https://github.com/xournalpp/xournalpp/issues/4377
             * some devices under some configurations can start an action while another one has already started
             * This is a workaround to avoid mem leaks and segfaults
             * This `if` statement can probably be replaced by an `assert` once #4377 is fixed
             */
            g_warning("InputHandler already exists upon XojPageView::onButtonPressEvent. Deleting it (and its views)");
            eraseViewsOf(this->overlayViews, this->inputHandler.get());
            this->inputHandler.reset();
        }

        Control* control = this->xournal->getControl();
        switch (h->getDrawingType()) {
            case DRAWING_TYPE_LINE:
                this->inputHandler = std::make_unique<RulerHandler>(control, getPage());
                break;
            case DRAWING_TYPE_RECTANGLE:
                this->inputHandler = std::make_unique<RectangleHandler>(control, getPage());
                break;
            case DRAWING_TYPE_ELLIPSE:
                this->inputHandler = std::make_unique<EllipseHandler>(control, getPage());
                break;
            case DRAWING_TYPE_ARROW:
                this->inputHandler = std::make_unique<ArrowHandler>(control, getPage(), false);
                break;
            case DRAWING_TYPE_DOUBLE_ARROW:
                this->inputHandler = std::make_unique<ArrowHandler>(control, getPage(), true);
                break;
            case DRAWING_TYPE_COORDINATE_SYSTEM:
                this->inputHandler = std::make_unique<CoordinateSystemHandler>(control, getPage());
                break;
            default:
                this->inputHandler = std::make_unique<StrokeHandler>(control, getPage());
        }
        this->inputHandler->onButtonPressEvent(pos, zoom);
        this->overlayViews.emplace_back(this->inputHandler->createView(this));

    } else if ((h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HIGHLIGHTER) &&
               h->getDrawingType() == DRAWING_TYPE_SPLINE) {
        if (!this->inputHandler) {
            this->inputHandler = std::make_unique<SplineHandler>(this->xournal->getControl(), getPage());
            this->inputHandler->onButtonPressEvent(pos, zoom);
            this->overlayViews.emplace_back(this->inputHandler->createView(this));
        } else {
            this->inputHandler->onButtonPressEvent(pos, zoom);
        }
    } else if (h->getToolType() == TOOL_ERASER) {
        this->eraser->erase(x, y);
        this->inEraser = true;
    } else if (h->getToolType() == TOOL_VERTICAL_SPACE) {
        if (this->verticalSpace) {
            control->getUndoRedoHandler()->addUndoAction(this->verticalSpace->finalize());
            this->verticalSpace.reset();
        }
        auto* zoomControl = this->getXournal()->getControl()->getZoomControl();
        this->verticalSpace = std::make_unique<VerticalToolHandler>(this->page, this->settings, y, pos.isControlDown());
        this->overlayViews.emplace_back(this->verticalSpace->createView(this, zoomControl, this->settings));
    } else if (h->getToolType() == TOOL_SELECT_RECT || h->getToolType() == TOOL_SELECT_REGION ||
               h->getToolType() == TOOL_SELECT_MULTILAYER_RECT || h->getToolType() == TOOL_SELECT_MULTILAYER_REGION ||
               h->getToolType() == TOOL_PLAY_OBJECT || h->getToolType() == TOOL_SELECT_OBJECT ||
               h->getToolType() == TOOL_SELECT_PDF_TEXT_LINEAR || h->getToolType() == TOOL_SELECT_PDF_TEXT_RECT) {
        if (h->getToolType() == TOOL_SELECT_RECT) {
            if (!selection) {
                this->selection = std::make_unique<RectSelection>(x, y);
                this->overlayViews.emplace_back(std::make_unique<xoj::view::SelectionView>(
                        this->selection.get(), this, this->settings->getSelectionColor()));
            } else {
                xoj_assert_message(
                        settings->getInputSystemTPCButtonEnabled(),
                        "the selection has already been created by a stylus button press while the stylus was "
                        "hovering!");
            }
        } else if (h->getToolType() == TOOL_SELECT_REGION) {
            if (!selection) {
                this->selection = std::make_unique<RegionSelect>(x, y);
                this->overlayViews.emplace_back(std::make_unique<xoj::view::SelectionView>(
                        this->selection.get(), this, this->settings->getSelectionColor()));
            } else {
                xoj_assert_message(
                        settings->getInputSystemTPCButtonEnabled(),
                        "the selection has already been created by a stylus button press while the stylus was "
                        "hovering!");
            }
        } else if (h->getToolType() == TOOL_SELECT_MULTILAYER_RECT) {
            if (!selection) {
                this->selection = std::make_unique<RectSelection>(x, y, /*multiLayer*/ true);
                this->overlayViews.emplace_back(std::make_unique<xoj::view::SelectionView>(
                        this->selection.get(), this, this->settings->getSelectionColor()));
            } else {
                xoj_assert_message(
                        settings->getInputSystemTPCButtonEnabled(),
                        "the selection has already been created by a stylus button press while the stylus was "
                        "hovering!");
            }
        } else if (h->getToolType() == TOOL_SELECT_MULTILAYER_REGION) {
            if (!selection) {
                this->selection = std::make_unique<RegionSelect>(x, y, /*multiLayer*/ true);
                this->overlayViews.emplace_back(std::make_unique<xoj::view::SelectionView>(
                        this->selection.get(), this, this->settings->getSelectionColor()));
            } else {
                xoj_assert_message(
                        settings->getInputSystemTPCButtonEnabled(),
                        "the selection has already been created by a stylus button press while the stylus was "
                        "hovering!");
            }
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
                auto sel = pdfToolbox->newSelection(x, y);
                this->overlayViews.emplace_back(
                        std::make_unique<xoj::view::PdfElementSelectionView>(sel, this, settings->getSelectionColor()));
            }
        } else if (h->getToolType() == TOOL_SELECT_OBJECT) {
            SelectObject select(this);
            if (pos.isShiftDown() && xournal->getSelection()) {
                select.atAggregate(x, y);
            } else {
                select.at(x, y);
            }
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
        // start selecting the size for the image
        this->imageSizeSelection = std::make_unique<ImageSizeSelection>(x, y);
        this->overlayViews.emplace_back(std::make_unique<xoj::view::ImageSizeSelectionView>(
                this->imageSizeSelection.get(), this, settings->getSelectionColor()));
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
                if (elems.size() > 1) {
                    // Deselect the other elements
                    this->xournal->clearSelection();
                    auto sel = SelectionFactory::createFromElementOnActiveLayer(control, getPage(), this, object);
                    this->xournal->setSelection(sel.release());
                }
                control->runLatex();
            }
        }
    } else if (toolType == TOOL_TEXT) {
        this->startText(x, y);
        this->textEditor->selectAtCursor(TextEditor::SelectType::WORD);
    } else if (toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT) {
        auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
        if (auto* selection = pdfToolbox->getSelection()) {
            pdfToolbox->selectionStyle = XojPdfPageSelectionStyle::Word;
            selection->currentPos(x, y, pdfToolbox->selectionStyle);
        }
    } else if (drawingType == DRAWING_TYPE_SPLINE) {
        if (this->inputHandler) {
            this->inputHandler->onButtonDoublePressEvent(pos, zoom);
            xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
            this->inputHandler.reset();
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
        this->textEditor->selectAtCursor(TextEditor::SelectType::PARAGRAPH);
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
    if (currentSequenceDeviceId && currentSequenceDeviceId != pos.deviceId) {
        // This motion event is not from the device which started the sequence: reject it
        return false;
    }

    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;

    ToolHandler* h = xournal->getControl()->getToolHandler();
    auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();

    if (this->inputHandler && this->inputHandler->onMotionNotifyEvent(pos, zoom)) {
        // input handler used this event
    } else if (this->imageSizeSelection) {
        this->imageSizeSelection->updatePosition(x, y);
    } else if (this->selection) {
        this->selection->currentPos(x, y);
    } else if (auto* selection = pdfToolbox->getSelection(); selection && !selection->isFinalized()) {
        selection->currentPos(x, y, pdfToolbox->selectionStyle);
    } else if (this->verticalSpace) {
        this->verticalSpace->currentPos(x, y);
    } else if (this->textEditor) {
        XournalppCursor* cursor = getXournal()->getCursor();
        cursor->setInvisible(false);

        const Text* text = this->textEditor->getTextElement();
        this->textEditor->mouseMoved(x - text->getX(), y - text->getY());
    } else if (h->getToolType() == TOOL_ERASER && h->getEraserType() != ERASER_TYPE_WHITEOUT && this->inEraser) {
        this->eraser->erase(x, y);
    }

    return false;
}

void XojPageView::onSequenceCancelEvent(DeviceId deviceId) {
    if (currentSequenceDeviceId != deviceId) {
        // This motion event is not from the device which started the sequence: reject it
        return;
    }
    currentSequenceDeviceId.reset();

    if (this->inputHandler) {
        this->inputHandler->onSequenceCancelEvent();

        if (auto* h = dynamic_cast<SplineHandler*>(this->inputHandler.get()); h) {
            // SplineHandler can survive a sequence cancellation
            if (!h->getStroke()) {
                xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
                this->inputHandler.reset();
            }
        } else {
            xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
            this->inputHandler.reset();
        }
    }
}

void XojPageView::onTapEvent(const PositionInputData& pos) {
    if (this->inputHandler) {
        /*
         * We only want tap events to trigger special actions if no tool is currently under use
         * (e.g. a spline is currently under edition)
         * Feed the event to the InputHandler as a click
         */
        const double zoom = getZoom();
        this->inputHandler->onButtonPressEvent(pos, zoom);
        this->inputHandler->onButtonReleaseEvent(pos, zoom);
        if (!this->inputHandler->getStroke()) {
            // The InputHandler finalized its edition
            xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
            this->inputHandler.reset();
        }
        return;
    }

    auto* settings = xournal->getControl()->getSettings();
    bool doAction = settings->getDoActionOnStrokeFiltered();
    if (settings->getTrySelectOnStrokeFiltered()) {
        double zoom = xournal->getZoom();
        SelectObject select(this);
        if (pos.isShiftDown()) {
            if (select.atAggregate(pos.x / zoom, pos.y / zoom)) {
                doAction = false;  // selection made.. no action.
            }
        } else {
            if (select.at(pos.x / zoom, pos.y / zoom)) {
                doAction = false;  // selection made.. no action.
            }
        }
    }

    if (doAction)  // pop up a menu
    {
        this->showFloatingToolbox(pos);
    }
}

auto XojPageView::showPdfToolbox(const PositionInputData& pos) -> void {
    // Compute coords of the canvas relative to the application window origin.
    gint wx = 0, wy = 0;
    GtkWidget* widget = xournal->getWidget();
    gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);

    // Add the position of the current page view widget (relative to canvas origin)
    // and add the input position (relative to the current page view widget).
    wx += this->getX() + round_cast<gint>(pos.x);
    wy += this->getY() + round_cast<gint>(pos.y);

    auto* pdfToolbox = this->xournal->getControl()->getWindow()->getPdfToolbox();
    pdfToolbox->show(wx, wy);
}

void XojPageView::deleteView(xoj::view::OverlayView* view) {
    auto it = std::find_if(this->overlayViews.begin(), this->overlayViews.end(),
                           [view](const auto& v) { return view == v.get(); });
    if (it != this->overlayViews.end()) {
        this->overlayViews.erase(it);
    }
}

auto XojPageView::onButtonReleaseEvent(const PositionInputData& pos) -> bool {
    if (currentSequenceDeviceId != pos.deviceId) {
        // This event is not from the device which started the sequence: reject it
        return false;
    }
    currentSequenceDeviceId.reset();

    Control* control = xournal->getControl();

    if (this->inputHandler) {
        double zoom = xournal->getZoom();
        this->inputHandler->onButtonReleaseEvent(pos, zoom);

        ToolHandler* h = control->getToolHandler();
        bool isDrawingTypeSpline = h->getDrawingType() == DRAWING_TYPE_SPLINE;
        if (!isDrawingTypeSpline || !this->inputHandler->getStroke()) {  // The Spline Tool finalizes drawing manually
            xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
            this->inputHandler.reset();
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
        this->verticalSpace.reset();
    }

    auto* pdfToolbox = control->getWindow()->getPdfToolbox();
    if (auto* selection = pdfToolbox->getSelection()) {
        if (!selection->isFinalized() && selection->finalizeSelectionAndRepaint(pdfToolbox->selectionStyle)) {
            // Selection was created, so reposition the toolbox and display.
            showPdfToolbox(pos);
        }
    }

    ToolType toolType = control->getToolHandler()->getActiveTool()->getToolType();
    if (xoj::tool::isPdfSelectionTool(toolType)) {
        const double zoom = xournal->getZoom();
        // Attempt PDF selection
        auto& pdfDoc = this->xournal->getDocument()->getPdfDocument();
        if (this->getPage()->getPdfPageNr() != npos) {
            auto page = pdfDoc.getPage(this->getPage()->getPdfPageNr());

            const double pageX = pos.x / zoom;
            const double pageY = pos.y / zoom;

            displayLinkPopover(page, pageX, pageY);
        }
    }

    if (this->selection) {
        const bool aggregate = pos.isShiftDown() && xournal->getSelection();
        size_t layerOfFinalizedSel = this->selection->finalize(this->page, aggregate, control->getDocument());

        if (layerOfFinalizedSel) {
            xournal->setSelection([&]() {
                if (aggregate) {
                    // Aggregate selection
                    auto sel = selection->releaseElements();
                    return SelectionFactory::addElementsFromActiveLayer(control, xournal->getSelection(), sel);
                } else {
                    // if selection->multiLayer == true, the selected objects might be on another layer
                    xournal->getControl()->getLayerController()->switchToLay(layerOfFinalizedSel);
                    return SelectionFactory::createFromElementsOnActiveLayer(control, page, this,
                                                                             selection->releaseElements());
                }
            }()
                                          .release());
        } else if (const double zoom = xournal->getZoom(); selection->userTapped(zoom)) {
            if (aggregate) {
                SelectObject(this).atAggregate(pos.x / zoom, pos.y / zoom);
            } else {
                SelectObject(this).at(pos.x / zoom, pos.y / zoom, this->selection->isMultiLayerSelection());
            }
        }
        this->selection.reset();
    } else if (this->textEditor) {
        this->textEditor->mouseReleased();
    }

    if (this->imageSizeSelection) {
        // size for image has been selected, now the image can be added
        auto spaceForImage = this->imageSizeSelection->getSelectedSpace();
        ImageHandler imgHandler(control, this);
        imgHandler.insertImageWithSize(spaceForImage);

        imageSizeSelection->finalize();
        this->imageSizeSelection.reset();
    }

    return false;
}

auto XojPageView::onKeyPressEvent(const KeyEvent& event) -> bool {
    if (this->textEditor) {
        if (this->textEditor->onKeyPressEvent(event)) {
            return true;
        }
    } else if (this->inputHandler) {
        if (this->inputHandler->onKeyPressEvent(event)) {
            return true;
        }
    } else if (this->verticalSpace) {
        if (this->verticalSpace->onKeyPressEvent(event)) {
            return true;
        }
    }

    // Esc leaves text edition
    if (event.keyval == GDK_KEY_Escape) {
        if (this->textEditor) {
            endText();
            return true;
        }

        return false;
    }

    return false;
}

auto XojPageView::onKeyReleaseEvent(const KeyEvent& event) -> bool {
    if (this->textEditor && this->textEditor->onKeyReleaseEvent(event)) {
        return true;
    }

    if (this->inputHandler && this->inputHandler->onKeyReleaseEvent(event)) {
        DrawingType drawingType = this->xournal->getControl()->getToolHandler()->getDrawingType();
        if (drawingType == DRAWING_TYPE_SPLINE) {  // Spline drawing has been finalized
            if (this->inputHandler) {
                xoj_assert(hasNoViewOf(overlayViews, inputHandler.get()));
                this->inputHandler.reset();
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
    xournal->getRepaintHandler()->repaintPageArea(this, floor_cast<int>(x1 * zoom), floor_cast<int>(y1 * zoom),
                                                  ceil_cast<int>(x2 * zoom), ceil_cast<int>(y2 * zoom));
}

void XojPageView::flagDirtyRegion(const Range& rg) const { repaintArea(rg.minX, rg.minY, rg.maxX, rg.maxY); }

void XojPageView::drawAndDeleteToolView(xoj::view::ToolView* v, const Range& rg) {
    if (v->isViewOf(this->inputHandler.get()) || v->isViewOf(this->verticalSpace.get()) ||
        v->isViewOf(this->textEditor.get())) {
        // Draw the inputHandler's view onto the page buffer.
        std::lock_guard lock(this->drawingMutex);
        v->drawWithoutDrawingAids(buffer.get());
    }
    this->deleteOverlayView(v, rg);
}

void XojPageView::deleteOverlayView(xoj::view::OverlayView* v, const Range& rg) {
    this->deleteView(v);
    if (!rg.empty()) {
        xoj_assert(rg.isValid());
        this->flagDirtyRegion(rg);
    }
}

double XojPageView::getZoom() const { return xournal->getZoom(); }

ZoomControl* XojPageView::getZoomControl() const { return this->getXournal()->getControl()->getZoomControl(); }

Range XojPageView::getVisiblePart() const {
    std::unique_ptr<xoj::util::Rectangle<double>> rect(xournal->getVisibleRect(this));
    if (rect) {
        return Range(*rect);
    }
    return Range();  // empty range
}

double XojPageView::getWidth() const { return page->getWidth(); }

double XojPageView::getHeight() const { return page->getHeight(); }

auto XojPageView::toWindowCoordinates(const xoj::util::Rectangle<double>& r) const -> xoj::util::Rectangle<double> {
    double zoom = this->getZoom();
    return {r.x * zoom + this->getX(), r.y * zoom + this->getY(), r.width * zoom, r.height * zoom};
}

void XojPageView::rerenderRect(double x, double y, double width, double height) {
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
    } else {
        this->endSpline();
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
        this->textEditor->copyToClipboard();
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

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, page->getWidth(), page->getHeight());
    cairo_fill(cr);

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

bool XojPageView::displayLinkPopover(std::shared_ptr<XojPdfPage> page, double pageX, double pageY) {
    // Search for selected link
    const auto links = page->getLinks();

    for (auto&& [rect, action]: links) {
        std::shared_ptr<const LinkDestination> dest = action->getDestination();

        if (!(rect.x1 <= pageX && pageX <= rect.x2 && rect.y1 <= pageY && pageY <= rect.y2)) {
            continue;
        }

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        GtkWidget* popover = makePopover(rect, box);

        if (auto uriOpt = dest->getURI()) {
            const std::string& uri = uriOpt.value();
            char* uriLabel = g_markup_escape_text(uri.c_str(), -1);

            auto labelMarkup = serdes_stream<std::stringstream>();
            labelMarkup << "<a href=" << std::quoted(uri) << ">" << uriLabel << "</a>";

            std::string linkMarkup = labelMarkup.str();

            g_free(uriLabel);

            GtkWidget* label = gtk_label_new(nullptr);
            gtk_label_set_markup(GTK_LABEL(label), linkMarkup.c_str());
            gtk_box_append(GTK_BOX(box), label);
        } else {
            size_t pdfPage = dest->getPdfPage();

            Document* doc = xournal->getControl()->getDocument();
            doc->lock();
            const size_t pageId = doc->findPdfPage(pdfPage);
            doc->unlock();

            GtkWidget* button{};
            if (pageId != npos) {
                const auto pageNo = static_cast<int64_t>(pageId + 1);
                button = gtk_button_new_with_label(FC(_F("Scroll to page {1}") % pageNo));
            } else {
                button = gtk_button_new_with_label(FC(_F("Add missing page")));
            }
            gtk_box_append(GTK_BOX(box), button);

            g_signal_connect(
                    button, "clicked",
                    G_CALLBACK(+[](GtkButton* bt,
                                   std::tuple<XojPageView*, std::shared_ptr<LinkDestination>, GtkWidget*>* state) {
                        XojPageView* self;
                        std::shared_ptr<LinkDestination> dest;
                        GtkWidget* popover;
                        std::tie(self, dest, popover) = *state;

                        self->getXournal()->getControl()->getScrollHandler()->scrollToLinkDest(*dest);
                        gtk_popover_popdown(GTK_POPOVER(popover));

                        delete state;
                    }),
                    new std::tuple(std::make_tuple(this, dest, popover)));
        }

        gtk_widget_show_all(popover);
        gtk_popover_popup(GTK_POPOVER(popover));
        return true;
    }

    return false;
}

GtkWidget* XojPageView::makePopover(const XojPdfRectangle& rect, GtkWidget* child) {
    double zoom = xournal->getZoom();

    GtkWidget* popover = gtk_popover_new(this->getXournal()->getWidget());
    gtk_popover_set_child(GTK_POPOVER(popover), child);

    auto x = floor_cast<int>(this->getX() + rect.x1 * zoom);
    auto y = floor_cast<int>(this->getY() + rect.y1 * zoom);
    auto w = ceil_cast<int>((rect.x2 - rect.x1) * zoom);
    auto h = ceil_cast<int>((rect.y2 - rect.y1) * zoom);

    GdkRectangle canvasRect{x, y, w, h};
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &canvasRect);
    gtk_popover_set_constrain_to(GTK_POPOVER(popover), GTK_POPOVER_CONSTRAINT_WINDOW);

    return popover;
}

auto XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect) -> bool {

    double zoom = xournal->getZoom();
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_scale(cr, zoom, zoom);

    {
        std::lock_guard lock(this->drawingMutex);  // Lock the mutex first
        xoj::util::CairoSaveGuard saveGuard(cr);   // see comment at the end of the scope
        if (!this->hasBuffer()) {
            drawLoadingPage(cr);
            return true;
        }

        if (this->buffer.getZoom() != zoom) {
            rerenderPage();
            cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
        }
        this->buffer.paintTo(cr);
    }  // Restore the state of cr and then release the mutex
       // restoring the state of cr ensures this->buffer.surface is not longer referenced as the source in cr.

    /**
     * All the overlay painters below follow the assumption:
     *  * The given cairo context is in page coordinates: no further scaling/offset is ever required.
     *
     * To anyone adding another painter here: please keep this assumption true
     */
    for (const auto& v: this->overlayViews) {
        v->draw(cr);
    }

    return true;
}

/**
 * GETTER / SETTER
 */

auto XojPageView::isSelected() const -> bool { return selected; }

auto XojPageView::hasBuffer() const -> bool { return this->buffer.isInitialized(); }

auto XojPageView::getSelectionColor() -> GdkRGBA { return Util::rgb_to_GdkRGBA(settings->getSelectionColor()); }

auto XojPageView::getTextEditor() -> TextEditor* { return textEditor.get(); }

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

auto XojPageView::getDisplayWidth() const -> int {
    return round_cast<int>(this->page->getWidth() * this->xournal->getZoom());
}

auto XojPageView::getDisplayHeight() const -> int {
    return round_cast<int>(this->page->getHeight() * this->xournal->getZoom());
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
    /*
     * The input handlers issue an elementChanged event when creating an element.
     * There is however no need to redraw the element in this case: the element was already painted to the buffer via a
     * call to drawAndDeleteToolView
     * There are a couple of exceptions:
     *  * if the added element is not on the top-most layer, a rerendering is required to draw it under the upper layers
     *  * if the added element overflows out of the visible part of the page, the ToolView may not have drawn to the
     *    page buffer the part outside the visible area. Rerendering as well
     */
    const bool noRerender = inputHandler && elem == inputHandler->getStroke() &&
                            page->getSelectedLayerId() == page->getLayerCount() &&
                            getVisiblePart().contains(elem->boundingRect());
    if (!noRerender) {
        rerenderElement(elem);
    }
}

void XojPageView::elementsChanged(const std::vector<Element*>& elements, const Range& range) {
    if (!range.empty()) {
        rerenderRange(range);
    }
}

void XojPageView::showFloatingToolbox(const PositionInputData& pos) {
    Control* control = xournal->getControl();

    gint wx = 0, wy = 0;
    GtkWidget* widget = xournal->getWidget();
    gtk_widget_translate_coordinates(widget, gtk_widget_get_toplevel(widget), 0, 0, &wx, &wy);

    wx += round_cast<int>(pos.x) + this->getX();
    wy += round_cast<int>(pos.y) + this->getY();

    control->getWindow()->getFloatingToolbox()->show(wx, wy);
}
