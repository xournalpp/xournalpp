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
    void prepareSave(const Document* doc);
    void saveTo(const fs::path& filepath, ProgressListener* listener = nullptr);
    void saveTo(OutputStream* out, const fs::path& filepath, ProgressListener* listener = nullptr);
    const std::string& getErrorMessage();

protected:
    static std::string getColorStr(Color c, unsigned char alpha = 0xff);

    virtual void visitPage(XmlNode* root, ConstPageRef p, const Document* doc, int id);
    virtual void visitLayer(XmlNode* page, const Layer* l);
    virtual void visitStroke(XmlPointNode* stroke, const Stroke* s);

    /**
     * Export the fill attributes
     */
    virtual void visitStrokeExtended(XmlPointNode* stroke, const Stroke* s);

    virtual void writeHeader();
    virtual void writeSolidBackground(XmlNode* background, ConstPageRef p);
    virtual void writeTimestamp(XmlAudioNode* xmlAudioNode, const AudioElement* audioElement);
    virtual void writeBackgroundName(XmlNode* background, ConstPageRef p);

protected:
    std::unique_ptr<XmlNode> root{};
    bool firstPdfPageVisited;
    int attachBgId;

    std::string errorMessage;

    std::vector<BackgroundImage> backgroundImages{};
};
