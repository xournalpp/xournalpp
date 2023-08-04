#include "ImageFrameEditor.h"

#include "control/Control.h"  // for Control
#include "control/settings/Settings.h"
#include "model/XojPage.h"        // for XojPage
#include "view/ImageFrameView.h"  // for ImageFrameView

ImageFrameEditor::ImageFrameEditor(Control* control, const PageRef& page, double x, double y):
        control(control), page(page), viewPool(std::make_shared<xoj::util::DispatchPool<ImageFrameEditorView>>()) {
    searchForImageFrame(x, y);
    Range new_box;
    new_box.addPoint(x, y);
    new_box.addPoint(x + 1, y + 1);
    box = new_box;
}

ImageFrameEditor::~ImageFrameEditor() {}

void ImageFrameEditor::mouseDown(double x, double y) {
    calculateScalingPosition(x, y);
    if (this->current != NO_SCALING && this->current != MOVE_IMAGE && this->current != PRE_MOVE_IMAGE) {
        this->scaling = true;
    } else {
        if (this->imageFrame != nullptr && this->current == PRE_MOVE_IMAGE) {
            this->current = MOVE_IMAGE;
            this->pos_buf.first = x;
            this->pos_buf.second = y;
            this->scaling = true;
        }
    }
}

void ImageFrameEditor::mouseMove(double x, double y) {
    updateImageFrame(x, y);

    if (this->scaling && this->imageFrame != nullptr) {
        const double frameX = this->imageFrame->getX();
        const double frameY = this->imageFrame->getY();
        const double frameWidth = this->imageFrame->getElementWidth();
        const double frameHeight = this->imageFrame->getElementHeight();
        const double frameXDif = x - frameX;
        const double frameYDif = y - frameY;

        switch (this->current) {
            case TOP:
                this->imageFrame->moveOnlyFrame(0, frameYDif, 0, -frameYDif);
                break;
            case TOP_RIGHT:
                this->imageFrame->moveOnlyFrame(0, frameYDif, frameXDif - frameWidth, -frameYDif);
                break;
            case TOP_LEFT:
                this->imageFrame->moveOnlyFrame(frameXDif, frameYDif, -frameXDif, -frameYDif);
                break;
            case BOTTOM:
                this->imageFrame->moveOnlyFrame(0, 0, 0, frameYDif - frameHeight);
                break;
            case BOTTOM_RIGHT:
                this->imageFrame->moveOnlyFrame(0, 0, frameXDif - frameWidth, frameYDif - frameHeight);
                break;
            case BOTTOM_LEFT:
                this->imageFrame->moveOnlyFrame(frameXDif, 0, -frameXDif, frameYDif - frameHeight);
                break;
            case LEFT:
                this->imageFrame->moveOnlyFrame(frameXDif, 0, -frameXDif, 0);
                break;
            case RIGHT:
                this->imageFrame->moveOnlyFrame(0, 0, frameXDif - frameWidth, 0);
                break;
            case MOVE_IMAGE:
                this->imageFrame->moveOnlyImage(x - this->pos_buf.first, y - this->pos_buf.second);
                this->pos_buf.first = x;
                this->pos_buf.second = y;
                break;
            case NO_SCALING:
                return;
        }
    }

    this->viewPool->dispatch(xoj::view::ImageFrameEditorView::FLAG_DIRTY_REGION, box);
    calculateRangeOfImageFrame();  // todo p0mm this seems overkill
    this->viewPool->dispatch(xoj::view::ImageFrameEditorView::FLAG_DIRTY_REGION, box);
}

void ImageFrameEditor::mouseUp(double x, double y) {
    this->scaling = false;
    if (this->current == MOVE_IMAGE) {
        this->current = NO_SCALING;
        this->scaling = false;
    }
}

auto ImageFrameEditor::searchForImageFrame(double x, double y) -> bool {
    for (Element* e: this->page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_IMAGEFRAME) {
            auto* imgF = dynamic_cast<ImageFrame*>(e);
            imgF->setCouldBeEdited(true);
            if (imgF->intersectsArea(x, y, 1, 1)) {
                if (this->imageFrame != nullptr) {
                    imageFrame->setInEditing(false);
                    this->page->fireElementChanged(imageFrame);
                }
                this->imageFrame = imgF;
                imgF->setInEditing(true);
                this->page->fireElementChanged(imgF);
                return true;
            }
        }
    }

    return false;
}

Color ImageFrameEditor::getSelectionColor() const { return this->control->getSettings()->getSelectionColor(); }

void ImageFrameEditor::updateImageFrame(double x, double y) {
    const std::vector<Element*> elems = this->page->getSelectedLayer()->getElements();
    if (std::find(elems.begin(), elems.end(), this->imageFrame) == elems.end()) {
        this->imageFrame = nullptr;
    }
    if (this->imageFrame == nullptr || (!this->imageFrame->intersectsArea(x, y, 1, 1) && !this->scaling)) {
        searchForImageFrame(x, y);
    }
}

auto ImageFrameEditor::getSelectionTypeForPos(double x, double y) -> CursorSelectionType {
    calculateScalingPosition(x, y);
    switch (this->current) {
        case TOP:
            return CURSOR_SELECTION_TOP;
            break;
        case TOP_RIGHT:
            return CURSOR_SELECTION_TOP_RIGHT;
            break;
        case TOP_LEFT:
            return CURSOR_SELECTION_TOP_LEFT;
            break;
        case BOTTOM:
            return CURSOR_SELECTION_BOTTOM;
            break;
        case BOTTOM_RIGHT:
            return CURSOR_SELECTION_BOTTOM_RIGHT;
            break;
        case BOTTOM_LEFT:
            return CURSOR_SELECTION_BOTTOM_LEFT;
            break;
        case LEFT:
            return CURSOR_SELECTION_LEFT;
            break;
        case RIGHT:
            return CURSOR_SELECTION_RIGHT;
            break;
        case NO_SCALING:
            return CURSOR_SELECTION_NONE;
            break;
        case MOVE_IMAGE:
            return CURSOR_SELECTION_NONE;
            break;
        case PRE_MOVE_IMAGE:
            return CURSOR_SELECTION_NONE;
            break;
    }
}

void ImageFrameEditor::calculateScalingPosition(double x, double y) {
    if (this->scaling || this->current == MOVE_IMAGE) {
        return;
    }
    updateImageFrame(x, y);
    if (this->imageFrame == nullptr) {
        this->current = NO_SCALING;
        return;
    }
    bool edgeLeft = true;
    bool edgeRight = true;
    bool edgeTop = true;
    bool edgeBottom = true;

    const double imageFrameLeft = this->imageFrame->getX();
    const double imageFrameRight = this->imageFrame->getX() + this->imageFrame->getElementWidth();
    const double imageFrameTop = this->imageFrame->getY();
    const double imageFrameBottom = this->imageFrame->getY() + this->imageFrame->getElementHeight();

    if (x + 3 < imageFrameLeft) {
        edgeLeft = false;
        edgeTop = false;
        edgeBottom = false;
    }
    if (x - 3 > imageFrameLeft) {
        edgeLeft = false;
    }
    if (x + 3 < imageFrameRight) {
        edgeRight = false;
    }
    if (x - 3 > imageFrameRight) {
        edgeRight = false;
        edgeTop = false;
        edgeBottom = false;
    }
    if (y + 3 < imageFrameTop) {
        edgeTop = false;
        edgeLeft = false;
        edgeRight = false;
    }
    if (y - 3 > imageFrameTop) {
        edgeTop = false;
    }
    if (y + 3 < imageFrameBottom) {
        edgeBottom = false;
    }
    if (y - 3 > imageFrameBottom) {
        edgeBottom = false;
        edgeLeft = false;
        edgeRight = false;
    }

    if (edgeLeft && edgeBottom) {
        this->current = BOTTOM_LEFT;
        return;
    } else if (edgeLeft && edgeTop) {
        this->current = TOP_LEFT;
        return;
    } else if (edgeRight && edgeBottom) {
        this->current = BOTTOM_RIGHT;
        return;
    } else if (edgeRight && edgeTop) {
        this->current = TOP_RIGHT;
        return;
    }
    if (edgeRight) {
        this->current = RIGHT;
        return;
    }
    if (edgeLeft) {
        this->current = LEFT;
        return;
    }
    if (edgeTop) {
        this->current = TOP;
        return;
    }
    if (edgeBottom) {
        this->current = BOTTOM;
        return;
    }

    if (this->imageFrame->getImagePosition().intersects({x, y, 1, 1})) {
        this->current = PRE_MOVE_IMAGE;
        return;
    }

    this->current = NO_SCALING;
}

auto ImageFrameEditor::currentlyScaling() -> bool { return this->current != NO_SCALING; }

void ImageFrameEditor::drawImageFrame(cairo_t* cr, double zoom) const {
    if (this->imageFrame == nullptr) {
        return;
    }
    auto elementView = xoj::view::ElementView::createFromElement(this->imageFrame);

    auto* imageFrameView = dynamic_cast<xoj::view::ImageFrameView*>(elementView.get());
    imageFrameView->setZoomForDrawing(zoom);
    imageFrameView->drawImage(cr, 0.3);
    imageFrameView->drawFrame(cr, getSelectionColor());
    imageFrameView->drawFrameHandles(cr, getSelectionColor());
}

void ImageFrameEditor::calculateRangeOfImageFrame() {
    Range new_box;
    if (this->imageFrame == nullptr) {
        return;
    } else if (imageFrame->hasImage()) {
        xoj::util::Rectangle<double> const imgPos = this->imageFrame->getImagePosition();
        new_box.addPoint(std::min(imgPos.x, this->imageFrame->getX() - 2),
                         std::min(imgPos.y, this->imageFrame->getY() - 2));
        new_box.addPoint(
                std::max(imgPos.x + imgPos.width, this->imageFrame->getX() + 2 + this->imageFrame->getElementWidth()),
                std::max(imgPos.y + imgPos.height,
                         this->imageFrame->getY() + 2 + this->imageFrame->getElementHeight()));
        box = new_box;
    } else {
        new_box.addPoint(this->imageFrame->getX() - 2, this->imageFrame->getY() - 2);
        new_box.addPoint(this->imageFrame->getX() + 2 + this->imageFrame->getElementWidth(),
                         this->imageFrame->getY() + 2 + this->imageFrame->getElementHeight());
        box = new_box;
    }
}

void ImageFrameEditor::resetView() {
    for (Element* e: this->page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_IMAGEFRAME) {
            auto* imgF = dynamic_cast<ImageFrame*>(e);
            imgF->setCouldBeEdited(false);
            if (imgF->inEditing()) {
                imgF->setInEditing(false);
            }
            this->page->fireElementChanged(imgF);
            box.addPoint(imgF->getX() - 2, imgF->getY() - 2);
            box.addPoint(imgF->getX() + imgF->getElementWidth() + 2, imgF->getY() + imgF->getElementHeight() + 2);
        }
    }
    Range bbox;
    bbox.addPoint(box.minX, box.minY);
    bbox.addPoint(box.maxX, box.maxY);
    this->viewPool->dispatchAndClear(xoj::view::ImageFrameEditorView::FINALIZATION_REQUEST, bbox);
}
