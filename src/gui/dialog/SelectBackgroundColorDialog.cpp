#include "SelectBackgroundColorDialog.h"

#include "control/Control.h"

#include "Util.h"
#include "i18n.h"

static inline std::array<GdkRGBA, 9> background1 = {
        Util::rgb_to_GdkRGBA(0xfabebeU),  //
        Util::rgb_to_GdkRGBA(0xfee7c4U),  //
        Util::rgb_to_GdkRGBA(0xfef8c9U),  //
        Util::rgb_to_GdkRGBA(0xdcf6c1U),  //
        Util::rgb_to_GdkRGBA(0xd4e2f0U),  //
        Util::rgb_to_GdkRGBA(0xe6d8e4U),  //
        Util::rgb_to_GdkRGBA(0xf8ead3U),  //
        Util::rgb_to_GdkRGBA(0xdadcdaU),  //
        Util::rgb_to_GdkRGBA(0xfafaf9U)   //
};

static inline std::array<GdkRGBA, 6> backgroundXournal = {
        Util::rgb_to_GdkRGBA(0xffffffU),  //
        Util::rgb_to_GdkRGBA(0xa0e8ffU),  //
        Util::rgb_to_GdkRGBA(0x80ffc0U),  //
        Util::rgb_to_GdkRGBA(0xffc0d4U),  //
        Util::rgb_to_GdkRGBA(0xffc080U),  //
        Util::rgb_to_GdkRGBA(0xffff80U)   //
};

SelectBackgroundColorDialog::SelectBackgroundColorDialog(Control* control):
        control(control),
        lastBackgroundColors{Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU),  //
                             Util::rgb_to_GdkRGBA(0xffffffU)}

{
    Settings* settings = control->getSettings();
    SElement& el = settings->getCustomElement("lastUsedPageBgColor");

    int count = 0;
    el.getInt("count", count);

    auto index = 0ULL;

    for (int i = 0; i < count && index < LAST_BACKGROUND_COLOR_COUNT; i++) {
        int iColor{};
        char* settingName = g_strdup_printf("color%02i", i);
        bool read = el.getInt(settingName, iColor);
        g_free(settingName);

        if (!read) {
            continue;
        }

        lastBackgroundColors[index] = Util::rgb_to_GdkRGBA(Color(iColor));

        ++index;
    }
}

void SelectBackgroundColorDialog::storeLastUsedValuesInSettings() {
    if (!this->selected) {  // No color selected, do not save to list
        return;
    }

    GdkRGBA newColor = Util::rgb_to_GdkRGBA(*this->selected);

    for (auto& lastBackgroundColor: lastBackgroundColors) {
        if (gdk_rgba_equal(&lastBackgroundColor, &newColor)) {
            // The new color is already in the list, do not save
            return;
        }
    }


    Settings* settings = control->getSettings();
    SElement& el = settings->getCustomElement("lastUsedPageBgColor");

    // Move all colors one step back
    for (int i = LAST_BACKGROUND_COLOR_COUNT - 1; i > 0; i--) {
        lastBackgroundColors[i] = lastBackgroundColors[i - 1];
    }

    lastBackgroundColors[0] = newColor;

    el.setInt("count", LAST_BACKGROUND_COLOR_COUNT);
    for (int i = 0; i < LAST_BACKGROUND_COLOR_COUNT; i++) {
        char* settingName = g_strdup_printf("color%02i", i);
        el.setIntHex(settingName, int(Util::GdkRGBA_to_argb(lastBackgroundColors[i])));
        g_free(settingName);
    }

    settings->customSettingsChanged();
}

auto SelectBackgroundColorDialog::getSelectedColor() const -> std::optional<Color> { return this->selected; }

void SelectBackgroundColorDialog::show(GtkWindow* parent) {
    GtkWidget* dialog = gtk_color_chooser_dialog_new(_("Select background color"), parent);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), false);

    gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9, background1.size(),
                                  background1.data());

    gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9, backgroundXournal.size(),
                                  backgroundXournal.data());

    // Last used colors (only by background, the general last used are shared with the
    // pen and highlighter etc.)
    gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(dialog), GTK_ORIENTATION_HORIZONTAL, 9, lastBackgroundColors.size(),
                                  lastBackgroundColors.data());


    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        GdkRGBA color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
        this->selected = Util::GdkRGBA_to_argb(color);

        storeLastUsedValuesInSettings();
    }

    gtk_widget_destroy(dialog);
}
