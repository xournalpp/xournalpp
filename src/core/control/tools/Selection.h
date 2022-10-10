/*
 * Xournal++
 *
 * A selection while you are selection, not for editing, only for selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include "model/Element.h"  // for Element (ptr only), ShapeContainer
#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef
#include "util/DispatchPool.h"
#include "util/Point.h"
#include "util/Range.h"
#include "view/overlays/SelectionView.h"

class Selection: public ShapeContainer, public OverlayBase {
public:
    Selection();
    ~Selection() override;

    using BoundaryPoint = utl::Point<double>;

public:
    virtual bool finalize(PageRef page) = 0;
    virtual void currentPos(double x, double y) = 0;
    virtual bool userTapped(double zoom) const = 0;
    virtual const std::vector<BoundaryPoint>& getBoundary() const = 0;

    inline const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SelectionView>>& getViewPool() const {
        return viewPool;
    }

private:
protected:
    std::vector<BoundaryPoint> boundaryPoints;

    std::vector<Element*> selectedElements;
    PageRef page;

    Range bbox;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SelectionView>> viewPool;

    friend class EditSelection;
};

class RectSelection: public Selection {
public:
    RectSelection(double x, double y);
    ~RectSelection() override;

public:
    bool finalize(PageRef page) override;
    void currentPos(double x, double y) override;
    bool contains(double x, double y) const override;
    bool userTapped(double zoom) const override;
    const std::vector<BoundaryPoint>& getBoundary() const override;

private:
    double sx;
    double sy;
    double ex;
    double ey;
    double maxDist = 0;
};

class RegionSelect: public Selection {
public:
    RegionSelect(double x, double y);

public:
    bool finalize(PageRef page) override;
    void currentPos(double x, double y) override;
    bool contains(double x, double y) const override;
    bool userTapped(double zoom) const override;
    const std::vector<BoundaryPoint>& getBoundary() const override;
};
