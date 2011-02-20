#include "ExportDialog.h"
#include "../../util/PageRange.h"

ExportDialog::ExportDialog() :
	GladeGui("export.glade", "exportDialog") {
}

ExportDialog::~ExportDialog() {
}

void ExportDialog::show() {
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

