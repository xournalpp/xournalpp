#include "ImageHandler.h"

#include <algorithm>  // for min
#include <memory>     // for __shared_ptr_access, make...
#include <string>     // for string
#include <utility>    // for operator==, pair

#include <glib-object.h>  // for g_object_unref
#include <glib.h>         // for g_error_free, g_free, GError

#include "control/Control.h"                // for Control
#include "control/stockdlg/ImageOpenDlg.h"  // for ImageOpenDlg
#include "control/tools/EditSelection.h"    // for EditSelection
#include "gui/MainWindow.h"                 // for MainWindow
#include "gui/PageView.h"                   // for XojPageView
#include "gui/XournalView.h"                // for XournalView
#include "model/Image.h"                    // for Image, Image::NOSIZE
#include "model/Layer.h"                    // for Layer
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "undo/InsertUndoAction.h"          // for InsertUndoAction
#include "undo/UndoRedoHandler.h"           // for UndoRedoHandler
#include "util/XojMsgBox.h"                 // for XojMsgBox
#include "util/i18n.h"                      // for _

ImageHandler::ImageHandler(Control* control, XojPageView* view) {
    this->control = control;
    this->view = view;
}

ImageHandler::~ImageHandler() = default;

auto ImageHandler::insertImage(double x, double y) -> bool {
    GFile* file = ImageOpenDlg::show(control->getGtkWindow(), control->getSettings());
    if (file == nullptr) {
        return false;
    }
    bool result = insertImage(file, x, y);
    g_object_unref(file);
    return result;
}

auto ImageHandler::insertImage(GFile* file, double x, double y) -> bool {
    Image* img = nullptr;
    {
        // Load the image data from disk
        GError* err = nullptr;
        gchar* contents{};
        gsize length{};
        if (!g_file_load_contents(file, nullptr, &contents, &length, nullptr, &err)) {
            g_error_free(err);
            return false;
        }

        img = new Image();
        img->setX(x);
        img->setY(y);
        img->setImage(std::string(contents, length));
        g_free(contents);
    }

    // Render the image.
    // FIXME: this is horrible. We need an ImageView class...
    (void)img->getImage();

    const auto imgSize = img->getImageSize();
    auto [width, height] = imgSize;
    if (imgSize == Image::NOSIZE) {
        delete img;
        XojMsgBox::showErrorToUser(this->control->getGtkWindow(),
                                   _("Failed to load image, could not determine image size!"));
        return false;
    }

    double zoom = 1;

    PageRef page = view->getPage();

    if (x + width > page->getWidth() || y + height > page->getHeight()) {
        double maxZoomX = (page->getWidth() - x) / width;
        double maxZoomY = (page->getHeight() - y) / height;
        zoom = std::min(maxZoomX, maxZoomY);
    }

    img->setWidth(width * zoom);
    img->setHeight(height * zoom);

    page->getSelectedLayer()->addElement(img);

    control->getUndoRedoHandler()->addUndoAction(
            std::make_unique<InsertUndoAction>(page, page->getSelectedLayer(), img));

    view->rerenderElement(img);
    auto* selection = new EditSelection(control->getUndoRedoHandler(), img, view, page);
    control->getWindow()->getXournal()->setSelection(selection);

    return true;
}
