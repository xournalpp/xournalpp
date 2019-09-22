#include "SelectBackgroundColorDialog.h"

#include "control/Control.h"

#include "Util.h"
#include "config.h"

#include <i18n.h>

GdkRGBA background1[] = {
        // clang-format off
        Util::rgb_to_GdkRGBA(0xfabebe),
        Util::rgb_to_GdkRGBA(0xfee7c4),
        Util::rgb_to_GdkRGBA(0xfef8c9),
        Util::rgb_to_GdkRGBA(0xdcf6c1),
        Util::rgb_to_GdkRGBA(0xd4e2f0),
        Util::rgb_to_GdkRGBA(0xe6d8e4),
        Util::rgb_to_GdkRGBA(0xf8ead3),
        Util::rgb_to_GdkRGBA(0xdadcda),
        Util::rgb_to_GdkRGBA(0xfafaf9)
        // clang-format on
};

const int background1Count = sizeof(background1) / sizeof(GdkRGBA);

// clang-format off
GdkRGBA backgroundXournal[] = {Util::rgb_to_GdkRGBA(0xffffff),
                               Util::rgb_to_GdkRGBA(0xa0e8ff),
                               Util::rgb_to_GdkRGBA(0x80ffc0),
                               Util::rgb_to_GdkRGBA(0xffc0d4),
                               Util::rgb_to_GdkRGBA(0xffc080),
                               Util::rgb_to_GdkRGBA(0xffff80)};
// clang-format on

const int backgroundXournalCount = sizeof(backgroundXournal) / sizeof(GdkRGBA);

SelectBackgroundColorDialog::SelectBackgroundColorDialog(Control* control)
 : control(control)  // clang-format off
 , lastBackgroundColors{Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff),
                        Util::rgb_to_GdkRGBA(0xffffff)}
// clang-format on
{
	Settings* settings = control->getSettings();
	SElement& el = settings->getCustomElement("lastUsedPageBgColor");

	int count = 0;
	el.getInt("count", count);

	int index = 0;

	for (int i = 0; i < count && index < LAST_BACKGROUND_COLOR_COUNT; i++)
	{
		int color = -1;
		char* settingName = g_strdup_printf("color%02i", i);
		bool read = el.getInt(settingName, color);
		g_free(settingName);

		if (!read)
		{
			continue;
		}

		lastBackgroundColors[index] = Util::rgb_to_GdkRGBA(color);

		index++;
	}
}

SelectBackgroundColorDialog::~SelectBackgroundColorDialog()
{
}

void SelectBackgroundColorDialog::storeLastUsedValuesInSettings()
{
	if (this->selected < 0)
	{
		// No color selected, do not save to list
		return;
	}

	GdkRGBA newColor = Util::rgb_to_GdkRGBA(this->selected);

	for (int i = 0; i < LAST_BACKGROUND_COLOR_COUNT; i++)
	{
		if (gdk_rgba_equal(&lastBackgroundColors[i], &newColor))
		{
			// The new color is already in the list, do not save
			return;
		}
	}


	Settings* settings = control->getSettings();
	SElement& el = settings->getCustomElement("lastUsedPageBgColor");

	// Move all colors one step back
	for (int i = LAST_BACKGROUND_COLOR_COUNT- 1; i > 0; i--)
	{
		lastBackgroundColors[i] = lastBackgroundColors[i - 1];
	}

	lastBackgroundColors[0] = newColor;

	el.setInt("count", LAST_BACKGROUND_COLOR_COUNT);
	for (int i = 0; i < LAST_BACKGROUND_COLOR_COUNT; i++)
	{
		char* settingName = g_strdup_printf("color%02i", i);
		el.setIntHex(settingName, Util::gdkrgba_to_hex(lastBackgroundColors[i]));
		g_free(settingName);
	}

	settings->customSettingsChanged();
}

int SelectBackgroundColorDialog::getSelectedColor()
{
	return this->selected;
}

void SelectBackgroundColorDialog::show(GtkWindow* parent)
{
	GtkWidget* dialog = gtk_color_chooser_dialog_new(_("Select background color"), parent);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), false);

	gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9,
			background1Count, background1);

	gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9,
			backgroundXournalCount, backgroundXournal);

	// Last used colors (only by background, the general last used are shared with the
	// pen and highlighter etc.)
	gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9,
			LAST_BACKGROUND_COLOR_COUNT, lastBackgroundColors);


	int response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK)
	{
		GdkRGBA color;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
		this->selected = Util::gdkrgba_to_hex(color);

		storeLastUsedValuesInSettings();
	}

	gtk_widget_destroy(dialog);
}
