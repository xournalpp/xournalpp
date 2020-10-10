/*
 * Xournal++
 *
 * Export a document for as .xoj compatible for Xournal,
 * remove all additional features which break the compatibility
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "SaveHandler.h"
#include "XournalType.h"

class XojExportHandler: public SaveHandler {
public:
    XojExportHandler();
    virtual ~XojExportHandler();

protected:
    /**
     * Export the fill attributes
     */
    void visitStrokeExtended(XmlPointNode* stroke, Stroke* s) override;
    void writeHeader() override;
    void writeSolidBackground(XmlNode* background, PageRef p) override;
    void writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode) override;
    void writeBackgroundName(XmlNode* background, PageRef p) override;

private:
};
