/*
 * Xournal++
 *
 * A job to export and publish PDF
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "PdfExportJob.h"

class PdfPublishJob: public PdfExportJob {
public:
    PdfPublishJob(Control* control, const string& script);
    void run();
protected:
    virtual ~PdfPublishJob();
private:
    string script;
};
