/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARCUSTOMIZEDIALOG_H__
#define __TOOLBARCUSTOMIZEDIALOG_H__

#include "../GladeGui.h"
#include "../../util/XournalType.h"

class ToolbarModel;
class ToolbarData;

class ToolbarCustomizeDialog : public GladeGui {
public:
	ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, ToolbarModel * model);
	virtual ~ToolbarCustomizeDialog();

public:
	virtual void show();

private:
	XOJ_TYPE_ATTRIB;

};

#endif /* __TOOLBARCUSTOMIZEDIALOG_H__ */
