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

    bool at(double x, double y, bool multiLayer = false) override {
        BaseSelectObject::at(x, y, multiLayer);

        if (match) {
            view->xournal->setSelection(
                    new EditSelection(view->xournal->getControl()->getUndoRedoHandler(), match, view, view->page));
            view->repaintPage();
            return true;
        }

        return false;
    }

    static constexpr double ACTION_RADIUS = 5.;

    bool checkLayer(const Layer* l) override {
        double minDistance = ACTION_RADIUS;
        // Iterate starting from the front-most element
        for (auto it = l->getElements().rbegin(); it < l->getElements().rend(); ++it) {
            // First perform a rough check to avoid expensive calls to Stroke::distanceTo()
            if ((*it)->intersectsArea(x - minDistance, y - minDistance, 2. * minDistance, 2. * minDistance)) {
                double d = (*it)->distanceTo(x, y);
                if (d == 0.0) {
                    this->match = *it;
                    return true;
                }
                if (d < minDistance) {
                    this->match = *it;
                    minDistance = d;
                    // Keep going, we may find something closer
                }
            }
        }
        return minDistance != ACTION_RADIUS;
    }

private:
    Element* match;
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
    static constexpr double ACTION_RADIUS = 15.;

    /// Plays every element of the layer that are closer than ACTION_RADIUS
    bool checkLayer(const Layer* l) override {
        bool found = false;
        for (auto&& e: l->getElements()) {
            if (auto* audio = dynamic_cast<AudioElement*>(e); audio) {
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
        return false;
    }
};
