#include "ImagesDialog.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <glib-object.h>

#include "gui/dialog/XojOpenDlg.h"  // for showOpenImageDialog
#include "gui/dialog/backgroundSelect/BackgroundSelectDialogBase.h"
#include "gui/dialog/backgroundSelect/BaseElementView.h"
#include "model/BackgroundImage.h"
#include "model/Document.h"
#include "model/PageRef.h"
#include "model/PageType.h"
#include "model/XojPage.h"
#include "util/PathUtil.h"
#include "util/Util.h"  // for npos
#include "util/XojMsgBox.h"
#include "util/i18n.h"

#include "ImageElementView.h"

class GladeSearchpath;
class Settings;

ImagesDialog::ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings,
                           std::function<void(BackgroundImage)> callback):
        BackgroundSelectDialogBase(gladeSearchPath, doc, settings, _("Select Image")), callback(std::move(callback)) {
    loadImagesFromPages();

    GtkWidget* lbl = gtk_label_new(_("<b>... or select already used Image:</b>"));
    gtk_label_set_use_markup(GTK_LABEL(lbl), true);
    gtk_box_prepend(vbox, lbl);

    fileChooserButton = GTK_BUTTON(gtk_button_new_with_label(_("Load file")));
    gtk_box_prepend(vbox, GTK_WIDGET(fileChooserButton));

    g_signal_connect(fileChooserButton, "clicked", G_CALLBACK(filechooserButtonCallback), this);

    g_signal_connect_swapped(
            okButton, "clicked", G_CALLBACK(+[](ImagesDialog* self) {
                if (self->selected < self->entries.size()) {
                    auto img = (static_cast<ImageElementView*>(self->entries[self->selected].get()))->backgroundImage;
                    if (!img.isEmpty()) {
                        self->callback(std::move(img));
                    }
                }
                gtk_window_close(self->window.get());
            }),
            this);

    populate();
}

ImagesDialog::~ImagesDialog() = default;

void ImagesDialog::loadImagesFromPages() {
    doc->lock();
    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);

        if (!p->getBackgroundType().isImagePage()) {
            continue;
        }

        if (p->getBackgroundImage().isEmpty()) {
            continue;
        }

        if (isImageAlreadyInTheList(p->getBackgroundImage())) {
            // Do not display the same image twice
            continue;
        }

        auto* iv = new ImageElementView(this->entries.size(), this);
        iv->backgroundImage = p->getBackgroundImage();
        this->entries.emplace_back(iv);
    }
    doc->unlock();
}

auto ImagesDialog::isImageAlreadyInTheList(BackgroundImage& image) -> bool {
    for (const auto& v: this->entries) {
        auto* iv = static_cast<ImageElementView*>(v.get());
        if (iv->backgroundImage == image) {
            return true;
        }
    }

    return false;
}

void ImagesDialog::filechooserButtonCallback(GtkButton*, ImagesDialog* dlg) {
    xoj::OpenDlg::showOpenImageDialog(dlg->window.get(), dlg->settings, [dlg](fs::path p, bool attach) {
        BackgroundImage img;
        GError* err = nullptr;
        img.loadFile(p, &err);
        img.setAttach(attach);
        if (err) {
            XojMsgBox::showErrorToUser(dlg->window.get(),
                                       FS(_F("This image could not be loaded. Error message: {1}") % err->message));
            g_error_free(err);
            return;
        }

        if (img.isEmpty()) {
            XojMsgBox::showErrorToUser(dlg->window.get(), _("This image could not be loaded."));
            return;
        }

        dlg->callback(std::move(img));
        gtk_window_close(dlg->window.get());
    });
}
