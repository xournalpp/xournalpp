#include "ToolbarDragDropHandler.h"

#include <glib.h>

#include "../../../control/Control.h"
#include "ToolbarDragDropHelper.h"
#include "../../toolbarMenubar/AbstractToolItem.h"
#include "ToolbarCustomizeDialog.h"

#include "ToolbarAdapter.h"

ToolbarDragDropHandler::ToolbarDragDropHandler() {
	XOJ_INIT_TYPE(ToolbarDragDropHandler);

	this->toolbars = NULL;
	this->customizeDialog = NULL;
}

ToolbarDragDropHandler::~ToolbarDragDropHandler() {
	XOJ_RELEASE_TYPE(ToolbarDragDropHandler);
}

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop(Control * control) {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	int len = 0;
	GtkWidget ** widgets = control->getWindow()->getToolbarWidgets(len);

	this->toolbars = new ToolbarAdapter*[len + 1];
	this->toolbars[len] = NULL;

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];
		this->toolbars[i] = new ToolbarAdapter(w, this);
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
}

void ToolbarDragDropHandler::configure(Control * control) {
	XOJ_CHECK_TYPE(ToolbarDragDropHandler);

	// remove original tool items
	ToolbarData * oldData = control->getWindow()->clearToolbar();

	this->prepareToolbarsForDragAndDrop(control);

	this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), control->getWindow());

	this->customizeDialog->show();

	delete this->customizeDialog;
	this->customizeDialog = NULL;

	this->clearToolbarsFromDragAndDrop();

	// restore original tool items
	control->getWindow()->loadToolbar(oldData);
}
