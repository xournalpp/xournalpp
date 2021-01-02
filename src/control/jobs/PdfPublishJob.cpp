#include "PdfPublishJob.h"

#include "control/Control.h"

#include "i18n.h"

PdfPublishJob::PdfPublishJob(Control* control, const string& script): PdfExportJob(control) {
    this->script = script;
}

PdfPublishJob::~PdfPublishJob() = default;

void PdfPublishJob::run() {
    PdfExportJob::run();
}
