#include "ImageHandler.h"

#include <memory>

#include "control/Control.h"
#include "control/stockdlg/ImageOpenDlg.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "undo/InsertUndoAction.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"

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
