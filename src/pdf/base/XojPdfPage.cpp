#include "XojPdfPage.h"


XojPdfRectangle::XojPdfRectangle()
{
    this->x1 = -1;
    this->x2 = -1;
    this->y1 = -1;
    this->y2 = -1;
}

XojPdfPage::XojPdfPage()
{
    XOJ_INIT_TYPE(XojPdfPage);
}

XojPdfPage::~XojPdfPage()
{
    XOJ_RELEASE_TYPE(XojPdfPage);
}
