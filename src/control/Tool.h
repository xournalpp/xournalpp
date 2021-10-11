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

#include <string>
#include <vector>

#include "ToolBase.h"


class Tool: public ToolBase {
public:
    Tool(std::string name, ToolType type, Color color, unsigned int capabilities, double* thickness);
    /**
     * @brief Construct a new Tool object based on another tool.
     * @param t tool to use as basis for new copy.
     */
    Tool(const Tool& t);
    virtual ~Tool();

    /**
     * @brief number of different sizes defined for tools with Size capability
     *
     */
    static constexpr int toolSizes = 5;

public:
    std::string getName() const;

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

    double* thickness;

    unsigned int capabilities;

    friend class ToolHandler;
};
