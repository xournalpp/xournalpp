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

using xoj::util::Rectangle;

ImageHandler::ImageHandler(Control* control, XojPageView* view): control(control), view(view) {}

ImageHandler::~ImageHandler() = default;


auto ImageHandler::chooseAndCreateImage(double x, double y) -> std::tuple<Image*, int, int> {
    const xoj::util::GObjectSPtr<GFile> fileObj(ImageOpenDlg::show(control->getGtkWindow(), control->getSettings()),
                                                xoj::util::adopt);
    if (!fileObj) {
        return std::make_tuple(nullptr, 0, 0);
    }

    GFile* file = fileObj.get();

    return createImageFromFile(file, x, y);
}


auto ImageHandler::createImageFromFile(GFile* file, double x, double y) -> std::tuple<Image*, int, int> {
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
    PageRef const page = view->getPage();

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

    PageRef const page = view->getPage();

    if (x + width > page->getWidth() || y + height > page->getHeight()) {
        double const maxZoomX = (page->getWidth() - x) / width;
        double const maxZoomY = (page->getHeight() - y) / height;
        zoom = std::min(maxZoomX, maxZoomY);
    }

    img->setWidth(width * zoom);
    img->setHeight(height * zoom);
}

auto ImageHandler::insertImageWithSize(Rectangle<double> space) -> bool {
    auto [img, width, height] = ImageHandler::chooseAndCreateImage(space.x, space.y);
    if (!img) {
        return false;
    }

    // todo p0mm choose between different options!
    // none option needs to set images own height and width!
    scaleImageUp(img, space);

    // make autoscaling toggleable by the user?
    automaticScaling(img, space.x, space.y,
                     img->getElementWidth() == 0.0 ? width : static_cast<int>(img->getElementWidth()),
                     img->getElementHeight() == 0.0 ? height : static_cast<int>(img->getElementHeight()));

    return addImageToDocument(img, true);
}

void ImageHandler::scaleImageDown(Image* img, xoj::util::Rectangle<double> space) {
    if (space.area() == 0) {
        return;
    }
    const double height = img->getElementHeight();
    const double width = img->getElementWidth();
    const double scaling = std::min(space.height / height, space.width / width);
    img->setWidth(scaling * width);
    img->setHeight(scaling * height);
}

void ImageHandler::scaleImageUp(Image* img, xoj::util::Rectangle<double> space) {
    if (space.area() == 0) {
        return;
    }
    const double height = img->getElementHeight();
    const double width = img->getElementWidth();
    const double scaling = std::max(space.height / height, space.width / width);
    img->setHeight(scaling * height);
    img->setWidth(scaling * width);
}
