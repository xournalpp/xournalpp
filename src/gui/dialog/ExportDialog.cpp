#include "ExportDialog.h"

#include <config.h>
#include <i18n.h>
#include <PageRange.h>

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath)
 : GladeGui(gladeSearchPath, "exportSettings.glade", "exportDialog")
{
	XOJ_INIT_TYPE(ExportDialog);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spDpi")), 300);

	g_signal_connect(get("rdRangePages"),
	                 "toggled",
	                 G_CALLBACK(+[](GtkToggleButton* togglebutton, ExportDialog* self) {
		                 XOJ_CHECK_TYPE_OBJ(self, ExportDialog);
		                 gtk_widget_set_sensitive(self->get("txtPages"), gtk_toggle_button_get_active(togglebutton));
	                 }),
	                 this);
}

ExportDialog::~ExportDialog()
{
	XOJ_CHECK_TYPE(ExportDialog);

	XOJ_RELEASE_TYPE(ExportDialog);
}

void ExportDialog::initPages(int current, int count)
{
	XOJ_CHECK_TYPE(ExportDialog);

	string allPages = "1 - " + std::to_string(count);
	gtk_label_set_text(GTK_LABEL(get("lbAllPagesInfo")), allPages.c_str());
	string currentPages = std::to_string(current);
	gtk_label_set_text(GTK_LABEL(get("lbCurrentPage")), currentPages.c_str());

	this->currentPage = current;
	this->pageCount = count;
}

void ExportDialog::removeDpiSelection()
{
	XOJ_CHECK_TYPE(ExportDialog);

	gtk_widget_hide(get("lbResolution"));
	gtk_widget_hide(get("spDpi"));
	gtk_widget_hide(get("lbDpi"));
}

int ExportDialog::getPngDpi()
{
	XOJ_CHECK_TYPE(ExportDialog);
	return gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spDpi")));
}

bool ExportDialog::isConfirmed()
{
	XOJ_CHECK_TYPE(ExportDialog);
	return this->confirmed;
}

PageRangeVector ExportDialog::getRange()
{
	XOJ_CHECK_TYPE(ExportDialog);

	GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
	GtkWidget* rdRangePages = get("rdRangePages");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages)))
	{
		return PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))));
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent)))
	{
		PageRangeVector range;
		range.push_back(new PageRangeEntry(this->currentPage, this->currentPage));
		return range;
	}
	else
	{
		PageRangeVector range;
		range.push_back(new PageRangeEntry(0, this->pageCount - 1));
		return range;
	}
}

void ExportDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(ExportDialog);

	confirmed = false;

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);

	int res = gtk_dialog_run(GTK_DIALOG(this->window));

	// Button 1 OK
	if (res == 1)
	{
		confirmed = true;
	}

	gtk_widget_hide(this->window);
}
