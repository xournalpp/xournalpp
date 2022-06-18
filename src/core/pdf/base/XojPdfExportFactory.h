/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

class Document;
class ProgressListener;
class XojPdfExport;

class XojPdfExportFactory {
private:
    XojPdfExportFactory();
    virtual ~XojPdfExportFactory();

public:
    static std::unique_ptr<XojPdfExport> createExport(Document* doc, ProgressListener* listener);

private:
};
