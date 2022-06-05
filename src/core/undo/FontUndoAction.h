/*
 * Xournal++
 *
 * Undo action for font changes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "UndoAction.h"


class FontUndoActionEntry;
class Layer;
class Text;
class XojFont;

class FontUndoAction: public UndoAction {
public:
    FontUndoAction(const PageRef& page, Layer* layer);
    ~FontUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

    void addStroke(Text* e, XojFont& oldFont, XojFont& newFont);

private:
    std::vector<FontUndoActionEntry*> data;

    Layer* layer;
};
