#include "ToolbarDragDropHandler.h"

#include <glib.h>

#include "../../../control/Control.h"
#include "ToolbarDragDropHelper.h"
#include "../../toolbarMenubar/AbstractToolItem.h"
#include "ToolbarCustomizeDialog.h"

#include "ToolbarAdapter.h"
#include "../../MainWindow.h"
#include "../../toolbarMenubar/model/ToolbarModel.h"

AbstractToolItem * ToolbarAdapter::currentDragItem = NULL;


ToolbarDragDropHandler::ToolbarDragDropHandler() {
	XOJ_INIT_TYPE(ToolbarDragDropHandler);

	this->toolbars = NULL;
	this->customizeDialog = NULL;
	this->refcount = 1;
}

ToolbarDragDropHandler::~ToolbarDragDropHandler() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	this->win->setInDragAndDropToolbar(true);

	XOJ_RELEASE_TYPE(ToolbarDragDropHandler);
}


void ToolbarDragDropHandler::ref() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	this->refcount++;
}

void ToolbarDragDropHandler::unref() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	this->refcount--;
	if(this->refcount == 0) {
		delete this;
	}
}

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop(Control * control) {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	int len = 0;
	MainWindow * win = control->getWindow();
	GtkWidget ** widgets = win->getToolbarWidgets(len);

	this->toolbars = new ToolbarAdapter*[len + 1];
	this->toolbars[len] = NULL;

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];
		this->toolbars[i] = new ToolbarAdapter(w, win->getToolbarName(GTK_TOOLBAR(w)), this, control->getWindow()->getToolMenuHandler(), control->getWindow());
	}
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	for (int i = 0; this->toolbars[i]; i++) {
		delete this->toolbars[i];
	}
	delete[] this->toolbars;

	this->toolbars = NULL;
}

void ToolbarDragDropHandler::toolbarDataChanged() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	this->customizeDialog->rebuildIconview();
	this->win->reloadToolbars();
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed() {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	delete this->customizeDialog;
	this->customizeDialog = NULL;

	this->clearToolbarsFromDragAndDrop();

	char * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, TOOLBAR_CONFIG, NULL);
	this->win->getToolbarModel()->save(file);
	g_free(file);

	this->unref();
}

void ToolbarDragDropHandler::configure(Control * control) {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	this->ref();

	this->win = control->getWindow();

	this->prepareToolbarsForDragAndDrop(control);

	this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), control->getWindow(), this);

	this->win->setInDragAndDropToolbar(true);

	this->customizeDialog->show();
}
