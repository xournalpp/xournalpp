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
}

ToolbarDragDropHandler::~ToolbarDragDropHandler()
{
	clearToolbarsFromDragAndDrop();
}

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop()
{
	int len = 0;
	MainWindow* win = control->getWindow();
	GtkWidget** widgets = win->getToolbarWidgets(len);

	this->toolbars = new ToolbarAdapter*[len + 1];
	this->toolbars[len] = nullptr;

	for (int i = 0; i < len; i++)
	{
		GtkWidget* w = widgets[i];
		this->toolbars[i] = new ToolbarAdapter(w, win->getToolbarName(GTK_TOOLBAR(w)),
											   control->getWindow()->getToolMenuHandler(), control->getWindow());
	}
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop()
{
	if (this->toolbars == nullptr)
	{
		return;
	}

	for (int i = 0; this->toolbars[i]; i++)
	{
		delete this->toolbars[i];
	}
	delete[] this->toolbars;

	this->toolbars = nullptr;
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed()
{
	delete this->customizeDialog;
	this->customizeDialog = nullptr;

	MainWindow* win = control->getWindow();

	this->clearToolbarsFromDragAndDrop();

	Path file = Util::getConfigFile(TOOLBAR_CONFIG);
	win->getToolbarModel()->save(file);
	win->floatingToolbox->hide();
}

void ToolbarDragDropHandler::configure()
{
	MainWindow* win = control->getWindow();
	
	

				
	win->floatingToolbox->showForConfiguration();

	this->prepareToolbarsForDragAndDrop();

	this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), win, this);

	this->customizeDialog->show(GTK_WINDOW(win->getWindow()));
}

bool ToolbarDragDropHandler::isInDragAndDrop()
{
	if (this->toolbars == nullptr)
	{
		return false;
	}

	return true;
}
