/*
 * Xournal++
 *
 * Helper classes to find / select objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

// No include needed, this is included after PageView.h

#include <limits>
#include <optional>

#include "control/AudioController.h"
#include "control/tools/EditSelection.h"
#include "util/PathUtil.h"

#include "XournalView.h"
#include "filesystem.h"

class BaseSelectObject {
public:
    BaseSelectObject(XojPageView* view): view(view) {}

    virtual ~BaseSelectObject() = default;

public:
    virtual bool at(double x, double y, bool multiLayer = false) {
        this->x = x;
        this->y = y;

        // clear old selection anyway
        view->xournal->getControl()->clearSelection();

        if (multiLayer) {
            size_t initialLayer = this->view->getPage()->getSelectedLayerId();
            for (int layerNo = static_cast<int>(this->view->getPage()->getLayers()->size() - 1); layerNo >= 0;
                 layerNo--) {
                Layer* layer = this->view->getPage()->getLayers()->at(layerNo);
                this->view->getXournal()->getControl()->getLayerController()->switchToLay(layerNo + 1);
                if (checkLayer(layer)) {
                    return true;
                }
            }
            this->view->getXournal()->getControl()->getLayerController()->switchToLay(initialLayer);
            return false;
        } else {
            Layer* layer = this->view->getPage()->getSelectedLayer();
            return checkLayer(layer);
        }
    }

protected:
    bool checkLayer(Layer* l) {
        /* Search for Element whose bounding box center is closest to the place (x,y) where the object is searched for.
         * Only those strokes are taken into account that pass the appropriate checkElement test.
         */
        bool found = false;
        double minDistSq = std::numeric_limits<double>::max();
        for (Element* e: l->getElements()) {
            const double eX = e->getX() + e->getElementWidth() / 2.0;
            const double eY = e->getY() + e->getElementHeight() / 2.0;
            const double dx = eX - this->x;
            const double dy = eY - this->y;
            const double distSq = dx * dx + dy * dy;
            const GdkRectangle matchRect = {gint(x - 10), gint(y - 10), 20, 20};
            if (e->intersectsArea(&matchRect) && distSq < minDistSq) {
                if (this->checkElement(e)) {
                    minDistSq = distSq;
                    found = true;
                }
            }
        }
        return found;
    }

    virtual bool checkElement(Element* e) = 0;

protected:
    XojPageView* view{};
    double x{0};
    double y{0};
};

class SelectObject: public BaseSelectObject {
public:
    SelectObject(XojPageView* view):
            BaseSelectObject(view), strokeMatch(nullptr), elementMatch(nullptr), gap(1000000000) {}

    ~SelectObject() override = default;

    bool at(double x, double y, bool multiLayer = false) override {
        BaseSelectObject::at(x, y, multiLayer);

        if (strokeMatch) {
            elementMatch = strokeMatch;
        }

        if (elementMatch) {
            view->xournal->setSelection(new EditSelection(view->xournal->getControl()->getUndoRedoHandler(),
                                                          elementMatch, view, view->page));

            view->repaintPage();

            return true;
        }

        return false;
    }

    bool atAggregate(double x, double y, bool multiLayer = false) {
        // If no previous elements, redirect to simple select
        if (view->xournal->getSelection() == nullptr) {
            return at(x, y, multiLayer);
        }

        // Store previous elements to aggregate them
        std::vector<Element*> allElements(view->xournal->getSelection()->getElements());

        BaseSelectObject::at(x, y, multiLayer);
        if (strokeMatch) {
            elementMatch = strokeMatch;
        }

        if (elementMatch) {
            if (std::find(allElements.begin(), allElements.end(), elementMatch) == allElements.end()) {
                allElements.push_back(elementMatch);
            }
            view->xournal->setSelection(new EditSelection(view->xournal->getControl()->getUndoRedoHandler(),
                                                          allElements, view, view->page));

            view->repaintPage();

            return true;
        } else {
            // No match, but we are aggregating, so it should not clear selection
            view->xournal->setSelection(new EditSelection(view->xournal->getControl()->getUndoRedoHandler(),
                                                          allElements, view, view->page));
            view->repaintPage();

            return false;
        }
    }

protected:
    bool checkElement(Element* e) override {
        if (e->getType() == ELEMENT_STROKE) {
            Stroke* s = (Stroke*)e;
            if (double sqDistance = s->isPointNearby(x, y, 25, gap * gap); sqDistance < gap * gap) {
                gap = std::sqrt(sqDistance);
                strokeMatch = s;
                return true;
            }
        } else {
            elementMatch = e;
            return true;
        }

        return false;
    }

private:
    Stroke* strokeMatch;
    Element* elementMatch;
    double gap;
};

class PlayObject: public BaseSelectObject {
public:
    PlayObject(XojPageView* view): BaseSelectObject(view), playbackStatus() {}

    ~PlayObject() override = default;

    struct Status {
        bool success;
        fs::path filename;
    };

    std::optional<Status> playbackStatus;

public:
    bool at(double x, double y, bool multiLayer = false) override { return BaseSelectObject::at(x, y, multiLayer); }

protected:
    bool checkElement(Element* e) override {
        if (e->getType() != ELEMENT_STROKE && e->getType() != ELEMENT_TEXT) {
            return false;
        }

        AudioElement* s = (AudioElement*)e;
        if (s->isPointNearby(x, y, 15, 60) < 60) {
            size_t ts = s->getTimestamp();

            if (auto fn = s->getAudioFilename(); !fn.empty()) {
                if (!fn.has_parent_path() || fs::weakly_canonical(fn.parent_path()) == "/") {
                    auto const& path = view->settings->getAudioFolder();
                    // Assume path exists
                    fn = path / fn;
                }
                auto* audioController = view->getXournal()->getControl()->getAudioController();
                if (!audioController) {
                    g_warning("Audio has been disabled");
                    return false;
                }
                bool success = audioController->startPlayback(fn, (unsigned int)ts);
                playbackStatus = {success, std::move(fn)};
                return success;
            }
        }
        return false;
    }
};
