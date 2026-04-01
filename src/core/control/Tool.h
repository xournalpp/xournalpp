/*
 * Xournal++
 *
 * A tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>     // for array
#include <optional>  // for optional
#include <string>    // for string

#include "control/ToolEnums.h"  // for ToolType, ToolCapabilities, ToolSize
#include "util/Color.h"         // for Color

#include "ToolBase.h"  // for ToolBase


class Tool: public ToolBase {
public:
    /**
     * @brief number of different sizes defined for tools with Size capability
     *
     */
    static constexpr int toolSizes = 5;

    Tool(std::string name, ToolType type, Color color, unsigned int capabilities,
         std::optional<std::array<double, Tool::toolSizes>> thickness);
    /**
     * @brief Construct a new Tool object based on another tool.
     * @param t tool to use as basis for new copy.
     */
    Tool(const Tool& t);
    ~Tool() override;

public:
    std::string getName() const;

    ToolType getToolType() const;

    bool hasCapability(ToolCapabilities cap) const;

    double getThickness(ToolSize size) const;

    /**
     * @brief Check whether the tool is a Drawing tool.
     * Drawing tools are considered all tools that directly change the canvas.
     * Right now these are:
     *  - Highlighter
     *  - Pen
     *  - Eraser
     *
     * @return true if tool is a drawing tool
     * @return false if tool is no drawing tool
     */
    bool isDrawingTool() const;

protected:
    void setCapability(unsigned int capability, bool enabled);

private:
    void operator=(const Tool& t);

private:
    std::string name;
    ToolType type;

    std::optional<std::array<double, toolSizes>> thickness;

    unsigned int capabilities;

    friend class ToolHandler;
};
