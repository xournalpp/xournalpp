/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLPAGESPINNER_H__
#define __TOOLPAGESPINNER_H__

#include "AbstractToolItem.h"
#include <XournalType.h>

class GladeGui;
class SpinPageAdapter;

class ToolPageSpinner : public AbstractToolItem
{
public:
    ToolPageSpinner(GladeGui* gui, ActionHandler* handler, string id,
                    ActionType type);
    virtual ~ToolPageSpinner();

public:
    SpinPageAdapter* getPageSpinner();
    void setText(string text);
    virtual string getToolDisplayName();

protected:
    virtual GtkToolItem* newItem();
    virtual GtkWidget* getNewToolIconImpl();

private:
    XOJ_TYPE_ATTRIB;

    GladeGui* gui;

    SpinPageAdapter* pageSpinner;
    GtkWidget* lbPageNo;
};

#endif /* __TOOLPAGESPINNER_H__ */
