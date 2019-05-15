#include "ToolbarDragDropHandler.h"

#include "ToolbarAdapter.h"
#include "ToolbarCustomizeDialog.h"
#include "ToolbarDragDropHelper.h"

#include "control/Control.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"

ToolbarDragDropHandler::ToolbarDragDropHandler(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(ToolbarDragDropHandler);
}

ToolbarDragDropHandler::~ToolbarDragDropHandler()
{
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	clearToolbarsFromDragAndDrop();

	XOJ_RELEASE_TYPE(ToolbarDragDropHandler);
}

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop()
{
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	int len = 0;
	MainWindow* win = control->getWindow();
	GtkWidget** widgets = win->getToolbarWidgets(len);

	this->toolbars = new ToolbarAdapter*[len + 1];
	this->toolbars[len] = NULL;

	for (int i = 0; i < len; i++)
	{
		GtkWidget* w = widgets[i];
		this->toolbars[i] = new ToolbarAdapter(w, win->getToolbarName(GTK_TOOLBAR(w)),
											   control->getWindow()->getToolMenuHandler(), control->getWindow());
	}
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop()
{
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	if (this->toolbars == NULL)
	{
		return;
	}

	for (int i = 0; this->toolbars[i]; i++)
	{
		delete this->toolbars[i];
	}
	delete[] this->toolbars;

	this->toolbars = NULL;
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed()
{
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	delete this->customizeDialog;
	this->customizeDialog = NULL;

	MainWindow* win = control->getWindow();

	this->clearToolbarsFromDragAndDrop();

	Path file = Util::getConfigFile(TOOLBAR_CONFIG);
	win->getToolbarModel()->save(file);
	win->hideFloatingToolbox();
}

void ToolbarDragDropHandler::configure()
{
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	MainWindow* win = control->getWindow();
	
	

				
	win->showFloatingToolboxForConfiguration();

	this->prepareToolbarsForDragAndDrop();

	this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), win, this);

	this->customizeDialog->show(GTK_WINDOW(win->getWindow()));
}

bool ToolbarDragDropHandler::isInDragAndDrop()
{
	if (this->toolbars == NULL)
	{
		return false;
	}

	return true;
}
