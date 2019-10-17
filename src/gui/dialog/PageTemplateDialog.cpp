#include "PageTemplateDialog.h"

#include "control/stockdlg/XojOpenDlg.h"
#include "gui/dialog/FormatDialog.h"
#include "gui/widgets/PopupMenuButton.h"
#include "model/FormatDefinitions.h"
#include "control/pagetype/PageTypeHandler.h"

#include <Util.h>
#include <PathUtil.h>

#include <config.h>
#include <i18n.h>

#include <sstream>
#include <fstream>
using std::ofstream;

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types)
 : GladeGui(gladeSearchPath, "pageTemplate.glade", "templateDialog"),
   settings(settings),
   pageMenu(new PageTypeMenu(types, settings, true, false))
{
	model.parse(settings->getPageTemplate());

	pageMenu->setListener(this);

	g_signal_connect(get("btChangePaperSize"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{ self->showPageSizeDialog(); }), this);

	g_signal_connect(get("btLoad"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{ self->loadFromFile(); }), this);

	g_signal_connect(get("btSave"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{ self->saveToFile(); }), this);

	popupMenuButton = new PopupMenuButton(get("btBackgroundDropdown"), pageMenu->getMenu());

	updateDataFromModel();
}

PageTemplateDialog::~PageTemplateDialog()
{
	delete pageMenu;
	pageMenu = nullptr;
	delete popupMenuButton;
	popupMenuButton = nullptr;
}

void PageTemplateDialog::updateDataFromModel()
{
	GdkRGBA color = Util::rgb_to_GdkRGBA(model.getBackgroundColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);

	updatePageSize();

	pageMenu->setSelected(model.getBackgroundType());

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage")), model.isCopyLastPageSettings());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize")), model.isCopyLastPageSize());
}

void PageTemplateDialog::changeCurrentPageBackground(PageTypeInfo* info)
{
	model.setBackgroundType(info->page);

	gtk_label_set_text(GTK_LABEL(get("lbBackgroundType")), info->name.c_str());
}

void PageTemplateDialog::saveToModel()
{
	model.setCopyLastPageSettings(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage"))));
	model.setCopyLastPageSize(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize"))));

	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);
	model.setBackgroundColor(Util::gdkrgba_to_hex(color));
}

void PageTemplateDialog::saveToFile()
{
	saveToModel();

	GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Save File"), GTK_WINDOW(this->getWindow()),
													GTK_FILE_CHOOSER_ACTION_SAVE, _("_Cancel"), GTK_RESPONSE_CANCEL,
													_("_Save"), GTK_RESPONSE_OK, nullptr);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal++ template"));
	gtk_file_filter_add_pattern(filterXoj, "*.xopt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

	if (!settings->getLastSavePath().isEmpty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}


	time_t curtime = time(nullptr);
	char stime[128];
	strftime(stime, sizeof(stime), "%F-Template-%H-%M.xopt", localtime(&curtime));
	string saveFilename = stime;

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return;
	}

	char* name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	string filename = name;
	char* folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);


	ofstream out;
	out.open(filename.c_str());
	out << model.toString();
	out.close();
}

void PageTemplateDialog::loadFromFile()
{
	XojOpenDlg dlg(GTK_WINDOW(this->getWindow()), this->settings);
	Path filename = dlg.showOpenTemplateDialog();

	string contents;
	if (!PathUtil::readString(contents, filename))
	{
		return;
	}
	model.parse(contents);

	updateDataFromModel();
}

void PageTemplateDialog::updatePageSize()
{
	const FormatUnits* formatUnit = &XOJ_UNITS[settings->getSizeUnitIndex()];

	char buffer[64];
	sprintf(buffer, "%0.2lf", model.getPageWidth() / formatUnit->scale);
	string pageSize = buffer;
	pageSize += formatUnit->name;
	pageSize += " x ";

	sprintf(buffer, "%0.2lf", model.getPageHeight() / formatUnit->scale);
	pageSize += buffer;
	pageSize += formatUnit->name;

	gtk_label_set_text(GTK_LABEL(get("lbPageSize")), pageSize.c_str());
}

void PageTemplateDialog::showPageSizeDialog()
{
	FormatDialog* dlg = new FormatDialog(getGladeSearchPath(), settings, model.getPageWidth(), model.getPageHeight());
	dlg->show(GTK_WINDOW(this->window));

	double width = dlg->getWidth();
	double height = dlg->getHeight();

	if (width > 0)
	{
		model.setPageWidth(width);
		model.setPageHeight(height);

		updatePageSize();
	}

	delete dlg;
}

/**
 * The dialog was confirmed / saved
 */
bool PageTemplateDialog::isSaved()
{
	return saved;
}

void PageTemplateDialog::show(GtkWindow* parent)
{
	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	int ret = gtk_dialog_run(GTK_DIALOG(this->window));

	if (ret == 1) // OK
	{
		saveToModel();
		settings->setPageTemplate(model.toString());

		this->saved = true;
	}

	gtk_widget_hide(this->window);
}
