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
#include <mutex>
#include <optional>

#include "control/AudioController.h"
#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"
#include "gui/PageView.h"
#include "model/Layer.h"
#include "model/XojPage.h"
#include "util/safe_casts.h"

#include "XournalView.h"

class BaseSelectObject {
public:
    BaseSelectObject(XojPageView* view): view(view) {}

    virtual ~BaseSelectObject() = default;

public:
    bool findAt(double x, double y, bool multiLayer = false, bool clearSelection = true) {
        this->x = x;
        this->y = y;

        Control* ctrl = this->view->getXournal()->getControl();

        if (clearSelection) {
            ctrl->clearSelection();
        }

        std::lock_guard lock(*ctrl->getDocument());
        if (multiLayer) {
            const auto& layers = this->view->getPage()->getLayers();
            size_t layerNo = layers.size();
            for (auto l = layers.rbegin(); l != layers.rend(); l++, layerNo--) {
                if (checkLayer(*l)) {
                    ctrl->getLayerController()->switchToLay(as_unsigned(std::distance(l, layers.rend())));
                    return true;
                }
            }
            return false;
        } else {
            Layer* layer = this->view->getPage()->getSelectedLayer();
            return checkLayer(layer);
        }
    }

protected:
    bool checkLayer(const Layer* l) {
        /* Search for Element whose bounding box center is closest to the place (x,y) where the object is searched for.
         * Only those strokes are taken into account that pass the appropriate checkElement test.
         */
        bool found = false;
        double minDistSq = std::numeric_limits<double>::max();
        Element::Index pos = 0;
        for (auto&& e: l->getElements()) {
            const double eX = e->getX() + e->getElementWidth() / 2.0;
            const double eY = e->getY() + e->getElementHeight() / 2.0;
            const double dx = eX - this->x;
            const double dy = eY - this->y;
            const double distSq = dx * dx + dy * dy;
            const GdkRectangle matchRect = {gint(x - 10), gint(y - 10), 20, 20};
            if (e->intersectsArea(&matchRect) && distSq < minDistSq) {
                if (this->checkElement(e.get(), pos)) {
                    minDistSq = distSq;
                    found = true;
                }
            }
            pos++;
        }
        return found;
    }

    virtual bool checkElement(Element* e, Element::Index pos) = 0;

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

    bool at(double x, double y, bool multiLayer = false) {
        findAt(x, y, multiLayer);

        if (strokeMatch) {
            elementMatch = strokeMatch;
        }

        if (elementMatch) {
            Control* ctrl = view->getXournal()->getControl();
            auto sel = SelectionFactory::createFromElementOnActiveLayer(ctrl, view->getPage(), view, elementMatch,
                                                                        matchIndex);
            view->xournal->setSelection(sel.release());

            view->repaintPage();

            return true;
        }

        return false;
    }

    bool atAggregate(double x, double y) {
        EditSelection* previousSelection = view->getXournal()->getSelection();
        xoj_assert(previousSelection);

        findAt(x, y, false, false);
        if (strokeMatch) {
            elementMatch = strokeMatch;
        }

        if (elementMatch) {
            auto sel = SelectionFactory::addElementFromActiveLayer(view->getXournal()->getControl(), previousSelection,
                                                                   elementMatch, matchIndex);
            view->getXournal()->setSelection(sel.release());

            view->repaintPage();

            return true;
        }
        return false;
    }

protected:
    bool checkElement(Element* e, Element::Index pos) override {
        if (e->getType() == ELEMENT_STROKE) {
            Stroke* s = (Stroke*)e;
            double tmpGap = 0;
            if ((s->intersects(x, y, 5, &tmpGap)) && (gap > tmpGap)) {
                gap = tmpGap;
                strokeMatch = s;
                matchIndex = pos;
                return true;
            }
        } else {
            elementMatch = e;
            matchIndex = pos;
            return true;
        }

        return false;
    }

private:
    Stroke* strokeMatch;
    Element* elementMatch;
    Element::Index matchIndex;
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
    bool at(double x, double y, bool multiLayer = false) { return findAt(x, y, multiLayer); }

protected:
    bool checkElement(Element* e, Element::Index) override {
        if (e->getType() != ELEMENT_STROKE && e->getType() != ELEMENT_TEXT) {
            return false;
        }

        AudioElement* s = (AudioElement*)e;
        double tmpGap = 0;
        if ((s->intersects(x, y, 15, &tmpGap))) {
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
