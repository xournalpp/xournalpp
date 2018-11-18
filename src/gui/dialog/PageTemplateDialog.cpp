#include "PageTemplateDialog.h"

#include "gui/dialog/FormatDialog.h"
#include "model/FormatDefinitions.h"

#include <Util.h>

#include <config.h>
#include <i18n.h>

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings)
 : GladeGui(gladeSearchPath, "pageTemplate.glade", "templateDialog"),
   settings(settings),
   saved(false)
{
	XOJ_INIT_TYPE(PageTemplateDialog);

	model.parse(settings->getPageTemplate());

	g_signal_connect(get("btChangePaperSize"), "clicked", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, PageTemplateDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, PageTemplateDialog);
			self->showPageSizeDialog();
		}), this);


	GdkRGBA color;
	Util::apply_rgb_togdkrgba(color, model.getBackgroundColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);

	updatePageSize();

	formatList.push_back({ .name = _("Plain"), .type = BACKGROUND_TYPE_NONE });
	formatList.push_back({ .name = _("Lined"), .type = BACKGROUND_TYPE_LINED });
	formatList.push_back({ .name = _("Ruled"), .type = BACKGROUND_TYPE_RULED });
	formatList.push_back({ .name = _("Graph"), .type = BACKGROUND_TYPE_GRAPH });

	GtkComboBoxText* cbBg = GTK_COMBO_BOX_TEXT(get("cbBackgroundFormat"));

	for (PageFormat& format: formatList)
	{
		gtk_combo_box_text_append_text(cbBg, format.name.c_str());
	}

	int activeFormat = 0;
	for (int i = 0; i < formatList.size(); i++)
	{
		if (formatList[i].type == model.getBackgroundType())
		{
			activeFormat = i;
			break;
		}
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbBg), activeFormat);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage")), model.isCopyLastPageSettings());
}

PageTemplateDialog::~PageTemplateDialog()
{
	XOJ_CHECK_TYPE(PageTemplateDialog);

	XOJ_RELEASE_TYPE(PageTemplateDialog);
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
		model.setCopyLastPageSettings(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage"))));

		GdkRGBA color;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);
		model.setBackgroundColor(Util::gdkrgba_to_hex(color));

		int activeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbBackgroundFormat")));
		model.setBackgroundType(formatList[activeIndex].type);

		settings->setPageTemplate(model.toString());
		this->saved = true;
	}

	gtk_widget_hide(this->window);
}
