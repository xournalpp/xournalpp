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
#include "XournalType.h"


class Tool: public ToolBase {
public:
    Tool(string name, ToolType type, Color color, int capabilities, double* thickness);
    /**
     * @brief Construct a new Tool object based on the pointer to another tool
     * Ideally this should be refactored to a copy constructor like Tool(const Tool& tool).
     * However, this would would require to much refactoring as of now.
     *
     * @param t tool to use as basis for new copy
     */
    Tool(Tool* t);
    virtual ~Tool();

    /**
     * @brief number of different sizes defined for tools with Size capability
     *
     */
    static const int toolSizes = 5;

public:
    string getName();

    bool hasCapability(ToolCapabilities cap) const;

    double getThickness(ToolSize size);

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
    bool isDrawingTool();

protected:
    void setCapability(int capability, bool enabled);

private:
    void operator=(const Tool& t);

private:
    string name;
    ToolType type;

    double* thickness;

    int capabilities;

    friend class ToolHandler;
};
