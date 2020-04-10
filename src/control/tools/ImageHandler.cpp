#include "ImageHandler.h"

#include <memory>

#include "control/Control.h"
#include "control/stockdlg/ImageOpenDlg.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "undo/InsertUndoAction.h"

#include "XojMsgBox.h"
#include "i18n.h"

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
    return insertImage(file, x, y);
}

auto ImageHandler::insertImage(GFile* file, double x, double y) -> bool {
    GError* err = nullptr;
    GFileInputStream* in = g_file_read(file, nullptr, &err);

    g_object_unref(file);

    GdkPixbuf* pixbuf = nullptr;

    if (!err) {
        pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), nullptr, &err);
        g_input_stream_close(G_INPUT_STREAM(in), nullptr, nullptr);
    } else {
        XojMsgBox::showErrorToUser(control->getGtkWindow(),
                                   FS(_F("This image could not be loaded. Error message: {1}") % err->message));
        g_error_free(err);
        return false;
    }

    auto* img = new Image();
    img->setX(x);
    img->setY(y);
    img->setImage(pixbuf);

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    g_object_unref(pixbuf);

    double zoom = 1;

    PageRef page = view->getPage();

    if (x + width > page->getWidth() || y + height > page->getHeight()) {
        double maxZoomX = (page->getWidth() - x) / width;
        double maxZoomY = (page->getHeight() - y) / height;

        if (maxZoomX < maxZoomY) {
            zoom = maxZoomX;
        } else {
            zoom = maxZoomY;
        }
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
