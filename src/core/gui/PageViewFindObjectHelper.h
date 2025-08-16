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
    virtual bool checkLayer(const Layer* l) = 0;

protected:
    XojPageView* view{};
    double x{0};
    double y{0};
};

class SelectObject: public BaseSelectObject {
public:
    SelectObject(XojPageView* view): BaseSelectObject(view), match(nullptr) {}

    ~SelectObject() override = default;

    bool at(double x, double y, bool multiLayer = false) {
        findAt(x, y, multiLayer);

        if (match) {
            Control* ctrl = view->getXournal()->getControl();
            auto sel = SelectionFactory::createFromElementOnActiveLayer(ctrl, view->getPage(), view, match, matchIndex);
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
        if (match) {
            auto sel = SelectionFactory::addElementFromActiveLayer(view->getXournal()->getControl(), previousSelection,
                                                                   match, matchIndex);
            view->getXournal()->setSelection(sel.release());
            view->repaintPage();

            return true;
        }
        return false;
    }

    static constexpr double ACTION_RADIUS = 5.;

    bool checkLayer(const Layer* l) override {
        double minDistance = ACTION_RADIUS;
        // Iterate starting from the front-most element
        Element::Index pos = as_signed(l->getElementsView().size());
        for (auto it = l->getElementsView().rbegin(); it < l->getElementsView().rend(); ++it) {
            pos--;
            // First perform a rough check to avoid expensive calls to Stroke::distanceTo()
            if ((*it)->intersectsArea(x - minDistance, y - minDistance, 2. * minDistance, 2. * minDistance)) {
                double d = (*it)->distanceTo(x, y);
                if (d == 0.0) {
                    this->match = *it;
                    this->matchIndex = pos;
                    return true;
                }
                if (d < minDistance) {
                    this->match = *it;
                    this->matchIndex = pos;
                    minDistance = d;
                    // Keep going, we may find something closer
                }
            }
        }
        return minDistance != ACTION_RADIUS;
    }

private:
    const Element* match;
    Element::Index matchIndex;
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
    static constexpr double ACTION_RADIUS = 15.;

    /// Plays every element of the layer that are closer than ACTION_RADIUS
    bool checkLayer(const Layer* l) override {
        bool found = false;
        for (auto&& e: l->getElementsView()) {
            if (auto* audio = dynamic_cast<const AudioElement*>(e); audio) {
                // First perform a rough check to avoid expensive calls to Stroke::distanceTo()
                if (audio->intersectsArea(x - ACTION_RADIUS, y - ACTION_RADIUS, 2. * ACTION_RADIUS,
                                          2. * ACTION_RADIUS)) {
                    double d = audio->distanceTo(x, y);
                    if (d < ACTION_RADIUS) {
                        found = playElement(audio) || found;
                    }
                }
            }
        }
        return found;
    }

    bool playElement(const AudioElement* s) {
#ifdef ENABLE_AUDIO
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
#else
        g_warning("Audio support was disabled at compile time");
#endif
        return false;
    }
};
