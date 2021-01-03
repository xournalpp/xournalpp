#include "PdfPublishJob.h"

#include "control/Control.h"

#include "i18n.h"

PdfPublishJob::PdfPublishJob(Control* control, const string& script): PdfExportJob(control) { this->script = script; }

PdfPublishJob::~PdfPublishJob() = default;

void PdfPublishJob::afterExport() {
    PdfExportJob::afterExport();

    if (script != "") {
        fs::path const target = this->filepath;
        string modscript(script);
        string parameter = "$1";
        size_t pos = 0;
        while (true) {
            pos = modscript.find(parameter, pos);
            if (pos == string::npos)
                break;
            modscript.erase(pos, parameter.length());
            modscript.insert(pos, string(target));
            pos += 1;
        }
        system(modscript.c_str());
    }
}
