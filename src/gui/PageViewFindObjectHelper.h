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

#include "util/audio/AudioPlayer.h"

#include "PathUtil.h"
#include "XournalView.h"
#include "filesystem.h"

class BaseSelectObject {
public:
    BaseSelectObject(XojPageView* view): view(view) {}

    virtual ~BaseSelectObject() = default;

public:
    virtual bool at(double x, double y) {
        this->x = x;
        this->y = y;

        // clear old selection anyway
        view->xournal->getControl()->clearSelection();
        matchRect = {gint(x - 10), gint(y - 10), 20, 20};

        Layer* layer = this->view->getPage()->getSelectedLayer();
        return checkLayer(layer);
    }

protected:
    bool checkLayer(Layer* l) {
        // Search for Element closest to center of matching rectangle
        bool found = false;
        double minDistSq = std::numeric_limits<double>::max();
        const double mX = matchRect.x + matchRect.width / 2.0;
        const double mY = matchRect.y + matchRect.height / 2.0;
        for (Element* e: *l->getElements()) {
            const double eX = e->getX() + e->getElementWidth() / 2.0;
            const double eY = e->getY() + e->getElementHeight() / 2.0;
            const double dx = eX - mX;
            const double dy = eY - mY;
            const double distSq = dx * dx + dy * dy;
            if (e->intersectsArea(&matchRect) && distSq < minDistSq) {
                minDistSq = distSq;
                if (this->checkElement(e)) {
                    found = true;
                }
            }
        }
        return found;
    }

    virtual bool checkElement(Element* e) = 0;

protected:
    GdkRectangle matchRect{};
    XojPageView* view{};
    double x{0};
    double y{0};
};

class SelectObject: public BaseSelectObject {
public:
    SelectObject(XojPageView* view):
            BaseSelectObject(view), strokeMatch(nullptr), elementMatch(nullptr), gap(1000000000) {}

    ~SelectObject() override = default;

    bool at(double x, double y) override {
        BaseSelectObject::at(x, y);

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

protected:
    bool checkElement(Element* e) override {
        if (e->getType() == ELEMENT_STROKE) {
            Stroke* s = (Stroke*)e;
            double tmpGap = 0;
            if ((s->intersects(x, y, 5, &tmpGap)) && (gap > tmpGap)) {
                gap = tmpGap;
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
        std::string filename;
    };

    std::optional<Status> playbackStatus;

public:
    bool at(double x, double y) override { return BaseSelectObject::at(x, y); }

protected:
    bool checkElement(Element* e) override {
        if (e->getType() != ELEMENT_STROKE && e->getType() != ELEMENT_TEXT) {
            return false;
        }

        AudioElement* s = (AudioElement*)e;
        double tmpGap = 0;
        if ((s->intersects(x, y, 15, &tmpGap))) {
            size_t ts = s->getTimestamp();

            string fn = s->getAudioFilename();

            if (!fn.empty()) {
                if (fn.rfind(G_DIR_SEPARATOR, 0) != 0) {
                    auto path = Util::fromUri(view->settings->getAudioFolder());

                    // Assume path exists
                    *path /= fn;

                    fn = path->string();
                }
                auto* ac = view->getXournal()->getControl()->getAudioController();
                bool success = ac->startPlayback(fn, (unsigned int)ts);
                playbackStatus = {success, fn};
                return success;
            }
        }
        return false;
    }
};
