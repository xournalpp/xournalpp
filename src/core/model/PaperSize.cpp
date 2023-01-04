#include "PaperSize.h"

PaperSize::PaperSize(const PaperFormatUtils::GtkPaperSizeUniquePtr_t& gtkPaperSize):
        width(gtk_paper_size_get_width(gtkPaperSize.get(), GTK_UNIT_POINTS)),
        height(gtk_paper_size_get_height(gtkPaperSize.get(), GTK_UNIT_POINTS)) {}

PaperSize::PaperSize(const PageTemplateSettings& model): width(model.getPageWidth()), height(model.getPageHeight()) {}

PaperSize::PaperSize(double width, double height): width(width), height(height) {}

auto PaperSize::orientation() const -> GtkOrientation { return static_cast<GtkOrientation>(height > width); }

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

auto PaperSize::areDoublesEqual(double x, double y) -> bool { return std::abs(x - y) <= 0.01; }
