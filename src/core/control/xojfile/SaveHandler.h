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

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "control/xml/XmlNode.h"    // for XmlNode
#include "model/BackgroundImage.h"  // for BackgroundImage
#include "model/PageRef.h"          // for PageRef
#include "util/Color.h"             // for Color

#include "filesystem.h"  // for path

class XmlPointNode;
class ProgressListener;
class AudioElement;
class Document;
class Layer;
class OutputStream;
class Stroke;
class XmlAudioNode;

class SaveHandler {
public:
    SaveHandler();

public:
    void prepareSave(Document* doc);
    void saveTo(const fs::path& filepath, ProgressListener* listener = nullptr);
    void saveTo(OutputStream* out, const fs::path& filepath, ProgressListener* listener = nullptr);
    std::string getErrorMessage();

protected:
    static std::string getColorStr(Color c, unsigned char alpha = 0xff);

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

    std::string errorMessage;

    std::vector<BackgroundImage> backgroundImages{};
};
