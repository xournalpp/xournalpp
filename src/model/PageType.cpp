#include "PageType.h"

PageType::PageType(): format(PageTypeFormat::Lined) {}

PageType::PageType(PageTypeFormat format): format(format) {}

PageType::PageType(const PageType& other) {
    this->format = other.format;
    this->config = other.config;
}

PageType::~PageType() = default;

/**
 * Compare Operator
 */
auto PageType::operator==(const PageType& other) const -> bool {
    return this->config == other.config && this->format == other.format;
}

/**
 * PDF background
 */
auto PageType::isPdfPage() const -> bool { return this->format == PageTypeFormat::Pdf; }

/**
 * Image Background
 */
auto PageType::isImagePage() const -> bool { return this->format == PageTypeFormat::Image; }

/**
 * Special background
 */
auto PageType::isSpecial() const -> bool {
    return this->format == PageTypeFormat::Pdf || this->format == PageTypeFormat::Image ||
           this->format == PageTypeFormat::Copy;
}
