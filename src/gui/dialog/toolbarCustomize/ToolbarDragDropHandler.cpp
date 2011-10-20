#include "ToolbarDragDropHandler.h"
#include "ToolbarAdapter.h"

#include <glib.h>

#include "../../../control/Control.h"
#include "ToolbarDragDropHelper.h"
#include "../../toolbarMenubar/AbstractToolItem.h"
#include "ToolbarCustomizeDialog.h"

#include "../../MainWindow.h"
#include "../../toolbarMenubar/model/ToolbarModel.h"

ToolbarDragDropHandler::ToolbarDragDropHandler(Control * control) {
	XOJ_INIT_TYPE(ToolbarDragDropHandler);

	this->control = control;
	this->toolbars = NULL;
	this->customizeDialog = NULL;
}

ToolbarDragDropHandler::~ToolbarDragDropHandler() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	clearToolbarsFromDragAndDrop();

	XOJ_RELEASE_TYPE(ToolbarDragDropHandler);
}

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	int len = 0;
	MainWindow * win = control->getWindow();
	GtkWidget ** widgets = win->getToolbarWidgets(len);

	this->toolbars = new ToolbarAdapter*[len + 1];
	this->toolbars[len] = NULL;

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];
		this->toolbars[i] = new ToolbarAdapter(w, win->getToolbarName(GTK_TOOLBAR(w)),
				control->getWindow()->getToolMenuHandler(), control->getWindow());
	}
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	if (this->toolbars == NULL) {
		return;
	}

	for (int i = 0; this->toolbars[i]; i++) {
		delete this->toolbars[i];
	}
	delete[] this->toolbars;

	this->toolbars = NULL;
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	delete this->customizeDialog;
	this->customizeDialog = NULL;

	printf("ToolbarDragDropHandler::toolbarConfigDialogClosed()\n");

	MainWindow * win = control->getWindow();

	this->clearToolbarsFromDragAndDrop();

	char * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, TOOLBAR_CONFIG,
			NULL);
	win->getToolbarModel()->save(file);
	g_free(file);
}

void ToolbarDragDropHandler::configure() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	MainWindow * win = control->getWindow();

	this->prepareToolbarsForDragAndDrop();

	this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), win, this);

	this->customizeDialog->show();
}

bool ToolbarDragDropHandler::isInDragAndDrop() {
	if (this->toolbars == NULL) {
		return false;
	}

	return true;
}

