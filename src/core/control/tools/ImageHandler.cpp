#include "ImageHandler.h"

#include <algorithm>  // for min
#include <fstream>
#include <memory>  // for __shared_ptr_access, make...
#include <sstream>
#include <string>   // for string
#include <utility>  // for operator==, pair

#include <glib-object.h>  // for g_object_unref
#include <glib.h>         // for g_error_free, g_free, GError

#include "control/Control.h"              // for Control
#include "control/tools/EditSelection.h"  // for EditSelection
#include "gui/MainWindow.h"               // for MainWindow
#include "gui/PageView.h"                 // for XojPageView
#include "gui/XournalView.h"              // for XournalView
#include "gui/dialog/XojOpenDlg.h"        // for showOpenImageDialog
#include "model/Image.h"
#include "model/Layer.h"            // for Layer
#include "model/PageRef.h"          // for PageRef
#include "model/XojPage.h"          // for XojPage
#include "undo/InsertUndoAction.h"  // for InsertUndoAction
#include "undo/UndoRedoHandler.h"   // for UndoRedoHandler
#include "util/XojMsgBox.h"         // for XojMsgBox
#include "util/i18n.h"              // for _
#include "util/raii/GObjectSPtr.h"  // for GObjectSPtr.h

ImageHandler::ImageHandler(Control* control): control(control) {}

ImageHandler::~ImageHandler() = default;


void ImageHandler::chooseAndCreateImage(std::function<void(std::unique_ptr<Image>)> callback) {
    xoj::OpenDlg::showOpenImageDialog(control->getGtkWindow(), control->getSettings(),
                                      [cb = std::move(callback), ctrl = control](fs::path p, bool) {
                                          auto img = ImageHandler::createImageFromFile(p);

                                          if (!img || img->getImageSize() == Image::NOSIZE) {
                                              XojMsgBox::showErrorToUser(ctrl->getGtkWindow(),
                                                                         _("Failed to load image"));
                                              return;
                                          }
                                          cb(std::move(img));
                                      });
}

auto ImageHandler::createImageFromFile(const fs::path& p) -> std::unique_ptr<Image> {

    auto fileToString = [](const fs::path& p) {
        // This is the faster file dump I could come up with. Faster by 20% than g_file_load_contents (with -O3)
        std::ifstream stream(p);

        auto pos = stream.tellg();
        stream.seekg(0, std::ios_base::end);  // Go to the end
        auto size = stream.tellg() - pos;     // Get the size
        stream.seekg(pos);                    // Go back

        std::string s(as_unsigned(size), 0);  // Allocate
        stream.read(&s[0], size);             // Dump
        return s;
    };

    auto img = std::make_unique<Image>();
    try {
        img->setImage(fileToString(p));
    } catch (const std::ios_base::failure& e) {
        std::stringstream msg;
        msg << _("Error while opening image file: ") << p.string() << '\n'
            << "Error code: " << e.code() << '\n'
            << "Explanatory string: " << e.what();
        XojMsgBox::showErrorToUser(nullptr, msg.str());
        return nullptr;
    }
    // Render the image.
    if (auto opt = img->renderBuffer(); opt.has_value()) {
        // An error occurred
        XojMsgBox::showErrorToUser(nullptr, opt.value());
        return nullptr;
    }

    return img;
}

bool ImageHandler::addImageToDocument(std::unique_ptr<Image> img, PageRef page, Control* control, bool addUndoAction) {
    Layer* layer = page->getSelectedLayer();

    if (addUndoAction) {
        control->getUndoRedoHandler()->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, img.get()));
    }

    XournalView* xournal = control->getWindow()->getXournal();
    auto pageNr = xournal->getCurrentPage();
    auto* view = xournal->getViewFor(pageNr);

    if (view->getPage() != page) {
        g_warning("Active page changed while you selected the image. Aborting.");
        return false;
    }

    auto sel = SelectionFactory::createFromFloatingElement(control, page, layer, view, std::move(img));
    control->getWindow()->getXournal()->setSelection(sel.release());
    return true;
}

void ImageHandler::automaticScaling(Image& img, PageRef page, int width, int height) {
    double zoom = 1;
    double x = img.getX();
    double y = img.getY();

    if (x + width > page->getWidth() || y + height > page->getHeight()) {
        double const maxZoomX = (page->getWidth() - x) / width;
        double const maxZoomY = (page->getHeight() - y) / height;
        zoom = std::min(maxZoomX, maxZoomY);
    }

    img.setWidth(width * zoom);
    img.setHeight(height * zoom);
}

void ImageHandler::automaticScaling(Image& img, PageRef page) {
    auto [width, height] = img.getImageSize();
    automaticScaling(img, page, width, height);
}


void ImageHandler::insertImageWithSize(PageRef page, const xoj::util::Rectangle<double>& space) {
    chooseAndCreateImage([space, page, ctrl = control](std::unique_ptr<Image> img) {
        xoj_assert(img);
        img->setX(space.x);
        img->setY(space.y);
        auto [width, height] = img->getImageSize();

        if (static_cast<int>(space.width) != 0 && static_cast<int>(space.height) != 0) {
            // scale down
            const double scaling = std::min(space.height / height, space.width / width);
            img->setWidth(scaling * width);
            img->setHeight(scaling * height);

            // center
            if (img->getElementHeight() < space.height) {
                img->setY(img->getY() + ((space.height - img->getElementHeight()) * 0.5));
            }
            if (img->getElementWidth() < space.width) {
                img->setX(img->getX() + ((space.width - img->getElementWidth()) * 0.5));
            }
        } else {
            // zero space is selected, scale original image size down to fit on the page
            automaticScaling(*img, page);
        }
        addImageToDocument(std::move(img), page, ctrl, true);
    });
}
