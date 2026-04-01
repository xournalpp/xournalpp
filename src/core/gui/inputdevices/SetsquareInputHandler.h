/*
 * Xournal++
 *
 * Input handler for the setsquare.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "GeometryToolInputHandler.h"

class Stroke;
class GeometryToolController;
struct InputEvent;

/**
 * @brief Input handler for the setsquare
 *
 * Only the pointer handling (mouse / stylus) is defined in this class.
 * The rest comes from the GeometryToolInputHandler
 * */
class SetsquareInputHandler: public GeometryToolInputHandler {

public:
    explicit SetsquareInputHandler(XournalView* xournalView, GeometryToolController* controller);
    ~SetsquareInputHandler() noexcept override;

private:
    /**
     * @brief handles input from mouse and stylus for the setsquare
     */
    bool handlePointer(InputEvent const& event) override;

    double getMinHeight() const override;
    double getMaxHeight() const override;
};
