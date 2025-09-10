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
#include "model/ElementInsertionPosition.h"
#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef
#include "util/DispatchPool.h"
#include "util/Point.h"
#include "util/Range.h"
#include "view/overlays/SelectorView.h"

class Document;

class Selector: public ShapeContainer, public OverlayBase {
public:
    Selector(bool multiLayer);
    ~Selector() override;

    using BoundaryPoint = xoj::util::Point<double>;

public:
    /**
     * @return layerId of selected objects, 0 if there is nothing in RectSelection
     */
    size_t finalize(PageRef page, bool disableMultilayer, Document* doc);

    virtual void currentPos(double x, double y) = 0;
    virtual bool userTapped(double zoom) const = 0;
    virtual const std::vector<BoundaryPoint>& getBoundary() const = 0;

    inline auto getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SelectorView>>& {
        return viewPool;
    }

    auto isMultiLayerSelection() -> bool;
    /**
     * Get the selected elements and clears them (std::move)
     */
    auto releaseElements() -> InsertionOrderRef;

private:
protected:
    std::vector<BoundaryPoint> boundaryPoints;

    bool multiLayer;

    InsertionOrderRef selectedElements;
    PageRef page;

    Range bbox;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SelectorView>> viewPool;

    friend class EditSelection;
};

class RectangularSelector: public Selector {
public:
    RectangularSelector(double x, double y, bool multiLayer = false);
    ~RectangularSelector() override;

public:
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

class LassoSelector: public Selector {
public:
    LassoSelector(double x, double y, bool multiLayer = false);

public:
    void currentPos(double x, double y) override;
    bool contains(double x, double y) const override;
    bool userTapped(double zoom) const override;
    const std::vector<BoundaryPoint>& getBoundary() const override;
};
