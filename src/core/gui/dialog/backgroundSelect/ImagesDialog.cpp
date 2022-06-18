#include "ImagesDialog.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <glib-object.h>

#include "gui/dialog/backgroundSelect/BackgroundSelectDialogBase.h"
#include "gui/dialog/backgroundSelect/BaseElementView.h"
#include "model/BackgroundImage.h"
#include "model/Document.h"
#include "model/PageRef.h"
#include "model/PageType.h"
#include "model/XojPage.h"

#include "ImageElementView.h"

class GladeSearchpath;
class Settings;


ImagesDialog::ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings):
        BackgroundSelectDialogBase(gladeSearchPath, doc, settings, "images.glade", "ImagesDialog") {
    loadImagesFromPages();

    g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK(okButtonCallback), this);
    g_signal_connect(get("btFilechooser"), "clicked", G_CALLBACK(filechooserButtonCallback), this);
}

ImagesDialog::~ImagesDialog() = default;

void ImagesDialog::loadImagesFromPages() {
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

        auto* iv = new ImageElementView(this->elements.size(), this);
        iv->backgroundImage = p->getBackgroundImage();
        this->elements.push_back(iv);
    }
}

auto ImagesDialog::isImageAlreadyInTheList(BackgroundImage& image) -> bool {
    for (BaseElementView* v: this->elements) {
        auto* iv = dynamic_cast<ImageElementView*>(v);
        if (iv->backgroundImage == image) {
            return true;
        }
    }

    return false;
}

void ImagesDialog::okButtonCallback(GtkButton* button, ImagesDialog* dlg) {
    dlg->confirmed = true;
    gtk_widget_hide(dlg->window);
}

void ImagesDialog::filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg) {
    dlg->selected = -2;
    dlg->confirmed = true;
    gtk_widget_hide(dlg->window);
}

auto ImagesDialog::shouldShowFilechooser() -> bool { return selected == -2 && confirmed; }

auto ImagesDialog::getSelectedImage() -> BackgroundImage {
    if (confirmed && selected >= 0 && selected < static_cast<int>(elements.size())) {
        return (dynamic_cast<ImageElementView*>(elements[selected]))->backgroundImage;
    }


    return BackgroundImage();
}

void ImagesDialog::show(GtkWindow* parent) {
    if (this->elements.empty()) {
        this->selected = -2;
        this->confirmed = true;
    } else {
        BackgroundSelectDialogBase::show(parent);
    }
}
