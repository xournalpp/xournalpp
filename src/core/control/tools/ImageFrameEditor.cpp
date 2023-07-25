#include "ImageFrameEditor.h"

#include "control/Control.h"  // for Control
#include "control/settings/Settings.h"
#include "model/XojPage.h"  // for XojPage

ImageFrameEditor::ImageFrameEditor(Control* control, const PageRef& page, double x, double y):
        control(control), page(page) {

    searchForImageFrame(x, y);
}

ImageFrameEditor::~ImageFrameEditor() {}

void ImageFrameEditor::mouseDown(double x, double y) {
    calculateScalingPosition(x, y);
    if (this->current != NO_SCALING) {
        this->scaling = true;
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
                this->imageFrame->scale(frameX, frameY, 1, (frameHeight - frameYDif) / frameHeight, 0.0, false);
                this->imageFrame->move(0, frameHeight - this->imageFrame->getElementHeight());
                break;
            case TOP_RIGHT:
                this->imageFrame->scale(frameX, frameY, frameXDif / frameWidth, (frameHeight - frameYDif) / frameHeight,
                                        0.0, false);
                this->imageFrame->move(0, frameHeight - this->imageFrame->getElementHeight());
                break;
            case TOP_LEFT:
                this->imageFrame->scale(frameX, frameY, (frameWidth - frameXDif) / frameWidth,
                                        (frameHeight - frameYDif) / frameHeight, 0.0, false);
                this->imageFrame->move(frameWidth - this->imageFrame->getElementWidth(),
                                       frameHeight - this->imageFrame->getElementHeight());
                break;
            case BOTTOM:
                this->imageFrame->scale(frameX, frameY, 1, frameYDif / frameHeight, 0.0, false);
                break;
            case BOTTOM_RIGHT:
                this->imageFrame->scale(frameX, frameY, frameXDif / frameWidth, frameYDif / frameHeight, 0.0, false);
                break;
            case BOTTOM_LEFT:
                this->imageFrame->scale(frameX, frameY, (frameWidth - frameXDif) / frameWidth, frameYDif / frameHeight,
                                        0.0, false);
                this->imageFrame->move(frameWidth - this->imageFrame->getElementWidth(), 0);
                break;
            case LEFT:
                this->imageFrame->scale(frameX, frameY, (frameWidth - frameXDif) / frameWidth, 1, 0.0, false);
                this->imageFrame->move(frameWidth - this->imageFrame->getElementWidth(), 0);
                break;
            case RIGHT:
                this->imageFrame->scale(frameX, frameY, frameXDif / frameWidth, 1, 0.0, false);
                break;
            case NO_SCALING:
                return;
        }
    }
}

void ImageFrameEditor::mouseUp(double x, double y) { this->scaling = false; }

bool ImageFrameEditor::searchForImageFrame(double x, double y) {
    for (Element* e: this->page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_IMAGEFRAME) {
            if (e->intersectsArea(x, y, 1, 1)) {
                this->imageFrame = dynamic_cast<ImageFrame*>(e);
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
    }
}

void ImageFrameEditor::calculateScalingPosition(double x, double y) {
    if (this->scaling) {
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
    this->current = NO_SCALING;
}

auto ImageFrameEditor::currentlyScaling() -> bool { return this->current != NO_SCALING; }
