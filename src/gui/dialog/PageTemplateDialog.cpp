#include "PageTemplateDialog.h"

#include "control/stockdlg/XojOpenDlg.h"
#include "gui/dialog/FormatDialog.h"
#include "model/FormatDefinitions.h"
#include "control/pagetype/PageTypeHandler.h"

#include <Util.h>

#include <config.h>
#include <i18n.h>

#include <fstream>
using std::ofstream;

static void menu_detacher(GtkWidget* widget, GtkMenu* menu)
{
	// Nothing to do
}

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types)
 : GladeGui(gladeSearchPath, "pageTemplate.glade", "templateDialog"),
   settings(settings),
   pageMenu(new PageTypeMenu(types, settings, false, false)),
   saved(false)
{
	XOJ_INIT_TYPE(PageTemplateDialog);

	model.parse(settings->getPageTemplate());

	pageMenu->setListener(this);

	g_signal_connect(get("btChangePaperSize"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, PageTemplateDialog);
			self->showPageSizeDialog();
		}), this);

	g_signal_connect(get("btLoad"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, PageTemplateDialog);
			self->loadFromFile();
		}), this);

	g_signal_connect(get("btSave"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, PageTemplateDialog);
			self->saveToFile();
		}), this);

	g_signal_connect(get("btBackgroundDropdown"), "clicked", G_CALLBACK(
		+[](GtkButton* button, PageTemplateDialog* self)
	{
			XOJ_CHECK_TYPE_OBJ(self, PageTemplateDialog);
			GtkWidget* menu = self->pageMenu->getMenu();

			// GTK 3.22: 
			gtk_menu_popup_at_widget(GTK_MENU(menu), GTK_WIDGET(button), GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL);

			gtk_menu_shell_select_first(GTK_MENU_SHELL(menu), FALSE);

		}), this);

	gtk_menu_attach_to_widget(GTK_MENU(pageMenu->getMenu()), get("btBackgroundDropdown"), menu_detacher);

	updateDataFromModel();
}

PageTemplateDialog::~PageTemplateDialog()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	delete pageMenu;
	pageMenu = NULL;

	XOJ_RELEASE_TYPE(PageTemplateDialog);
}

void PageTemplateDialog::updateDataFromModel()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	GdkRGBA color;
	Util::apply_rgb_togdkrgba(color, model.getBackgroundColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);

	updatePageSize();

	pageMenu->setSelected(model.getBackgroundType());

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage")), model.isCopyLastPageSettings());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize")), model.isCopyLastPageSize());
}

void PageTemplateDialog::pageSelected(PageTypeInfo* info)
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	model.setBackgroundType(info->page);

	gtk_label_set_text(GTK_LABEL(get("lbBackgroundType")), info->name.c_str());
}

void PageTemplateDialog::saveToModel()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	model.setCopyLastPageSettings(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage"))));
	model.setCopyLastPageSize(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize"))));

	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);
	model.setBackgroundColor(Util::gdkrgba_to_hex(color));
}

void PageTemplateDialog::saveToFile()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	saveToModel();

	GtkWidget* dialog = gtk_file_chooser_dialog_new(_C("Save File"), GTK_WINDOW(this->getWindow()),
													GTK_FILE_CHOOSER_ACTION_SAVE, _C("_Cancel"), GTK_RESPONSE_CANCEL,
													_C("_Save"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _C("Xournal++ template"));
	gtk_file_filter_add_pattern(filterXoj, "*.xopt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

	if (!settings->getLastSavePath().empty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), PATH_TO_CSTR(settings->getLastSavePath()));
	}


	time_t curtime = time(NULL);
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
	XOJ_CHECK_TYPE(PageTemplateDialog);

	XojOpenDlg dlg(GTK_WINDOW(this->getWindow()), this->settings);
	path filename = dlg.showOpenTemplateDialog();

	std::ifstream file(filename.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();

	model.parse(buffer.str());

	updateDataFromModel();
}

void PageTemplateDialog::updatePageSize()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

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
	XOJ_CHECK_TYPE(PageTemplateDialog);

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
	XOJ_CHECK_TYPE(PageTemplateDialog);

	return saved;
}

void PageTemplateDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

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
