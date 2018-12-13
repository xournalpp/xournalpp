#include "XojPdfExport.h"

XojPdfExport::XojPdfExport()
{
    XOJ_INIT_TYPE(XojPdfExport);
}

XojPdfExport::~XojPdfExport()
{
    XOJ_RELEASE_TYPE(XojPdfExport);
}

/**
 * Export without background
 */
void XojPdfExport::setNoBackgroundExport(bool noBackgroundExport)
{
	XOJ_CHECK_TYPE(XojPdfExport);
	// Does nothing in the base class
}
