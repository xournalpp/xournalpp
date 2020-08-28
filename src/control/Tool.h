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
    virtual ~Tool();

public:
    string getName();

    bool hasCapability(ToolCapabilities cap) const;

    double getThickness(ToolSize size);

protected:
    void setCapability(int capability, bool enabled);

private:
    Tool(const Tool& t);
    void operator=(const Tool& t);

private:
    string name;
    ToolType type;

    double* thickness;

    int capabilities;

    friend class ToolHandler;
};
