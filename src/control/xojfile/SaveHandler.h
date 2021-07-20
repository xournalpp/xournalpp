/*
 * Xournal++
 *
 * Saves a document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "control/xml/XmlAudioNode.h"
#include "model/Document.h"
#include "model/PageRef.h"
#include "model/Stroke.h"

#include "OutputStream.h"
#include "XournalType.h"

class XmlNode;
class XmlPointNode;
class ProgressListener;

class SaveHandler {
public:
    SaveHandler();

public:
    void prepareSave(Document* doc);
    void saveTo(const fs::path& filepath, ProgressListener* listener = nullptr);
    void saveTo(OutputStream* out, const fs::path& filepath, ProgressListener* listener = nullptr);
    string getErrorMessage();

protected:
    static string getColorStr(Color c, unsigned char alpha = 0xff);

    virtual void visitPage(XmlNode* root, PageRef p, Document* doc, int id);
    virtual void visitLayer(XmlNode* page, Layer* l);
    virtual void visitStroke(XmlPointNode* stroke, Stroke* s);

    /**
     * Export the fill attributes
     */
    virtual void visitStrokeExtended(XmlPointNode* stroke, Stroke* s);

    virtual void writeHeader();
    virtual void writeSolidBackground(XmlNode* background, PageRef p);
    virtual void writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode);
    virtual void writeBackgroundName(XmlNode* background, PageRef p);

protected:
    std::unique_ptr<XmlNode> root{};
    bool firstPdfPageVisited;
    int attachBgId;

    string errorMessage;

    std::vector<BackgroundImage> backgroundImages{};
};
