#include "PaperSize.h"

static auto areDoublesEqual(double x, double y) -> bool { return std::abs(x - y) <= 0.01; }

PaperSize::PaperSize(const xoj::util::GtkPaperSizeUPtr& gtkPaperSize):
        width(gtk_paper_size_get_width(gtkPaperSize.get(), GTK_UNIT_POINTS)),
        height(gtk_paper_size_get_height(gtkPaperSize.get(), GTK_UNIT_POINTS)) {}

PaperSize::PaperSize(const PageTemplateSettings& model): width(model.getPageWidth()), height(model.getPageHeight()) {}

PaperSize::PaperSize(double width, double height): width(width), height(height) {}

auto PaperSize::orientation() const -> PaperOrientation { return static_cast<PaperOrientation>(height > width); }

void PaperSize::swapWidthHeight() {
    double const originalWidth = width;
    width = height;
    height = originalWidth;
}

auto PaperSize::equalDimensions(const PaperSize& other) const -> bool {
    return (areDoublesEqual(other.height, height) && areDoublesEqual(other.width, width)) ||
           (areDoublesEqual(other.height, width) && areDoublesEqual(other.width, height));
}

auto PaperSize::operator!=(const PaperSize& other) const -> bool { return !operator==(other); }

auto PaperSize::operator==(const PaperSize& other) const -> bool {
    return areDoublesEqual(other.height, height) && areDoublesEqual(other.width, width);
}
