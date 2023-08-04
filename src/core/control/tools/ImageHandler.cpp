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
#include "model/Layer.h"                    // for Layer
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "undo/InsertUndoAction.h"          // for InsertUndoAction
#include "undo/UndoRedoHandler.h"           // for UndoRedoHandler
#include "util/XojMsgBox.h"                 // for XojMsgBox
#include "util/i18n.h"                      // for _
#include "util/raii/GObjectSPtr.h"          // for GObjectSPtr.h

ImageHandler::ImageHandler(Control* control, XojPageView* view) {
    this->control = control;
    this->view = view;
}

ImageHandler::~ImageHandler() = default;

auto ImageHandler::insertImage(double x, double y) -> bool {
    xoj::util::GObjectSPtr<GFile> file(ImageOpenDlg::show(control->getGtkWindow(), control->getSettings()),
                                       xoj::util::adopt);
    if (!file) {
        return false;
    }
    return insertImage(file.get(), x, y);
}

auto ImageHandler::createImage(GFile* file, double x, double y) -> std::tuple<Image*, int, int> {
    Image* img = nullptr;
    {
        // Load the image data from disk
        GError* err = nullptr;
        gchar* contents{};
        gsize length{};
        if (!g_file_load_contents(file, nullptr, &contents, &length, nullptr, &err)) {
            g_error_free(err);
            return std::make_tuple(nullptr, 0, 0);
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
        return std::make_tuple(nullptr, 0, 0);
    }

    return std::make_tuple(img, width, height);
}

auto ImageHandler::addImageToDocument(Image* img, bool addUndoAction) -> bool {
    PageRef page = view->getPage();

    page->getSelectedLayer()->addElement(img);

    if (addUndoAction) {
        control->getUndoRedoHandler()->addUndoAction(
                std::make_unique<InsertUndoAction>(page, page->getSelectedLayer(), img));
    }

    view->rerenderElement(img);
    auto* selection = new EditSelection(control->getUndoRedoHandler(), img, view, page);
    control->getWindow()->getXournal()->setSelection(selection);

    return true;
}

void ImageHandler::automaticScaling(Image* img, double x, double y, int width, int height) {
    double zoom = 1;

    PageRef page = view->getPage();

    if (x + width > page->getWidth() || y + height > page->getHeight()) {
        double maxZoomX = (page->getWidth() - x) / width;
        double maxZoomY = (page->getHeight() - y) / height;
        zoom = std::min(maxZoomX, maxZoomY);
    }

    img->setWidth(width * zoom);
    img->setHeight(height * zoom);
}

auto ImageHandler::insertImage(GFile* file, double x, double y) -> bool {
    auto [img, width, height] = ImageHandler::createImage(file, x, y);
    if (!img) {
        return false;
    }
    automaticScaling(img, x, y, width, height);
    return addImageToDocument(img, true);
}
