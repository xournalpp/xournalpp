#include "ToolbarCustomizeDialog.h"
#include "../toolbarMenubar/model/ToolbarData.h"
#include "../toolbarMenubar/model/ToolbarModel.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, ToolbarModel * model) :
	GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);

}

void ToolbarCustomizeDialog::show() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}
