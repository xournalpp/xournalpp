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
#include <optional>
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
    /// Prepare an XML tree corresponding to the document - Needs read-only access to the Document
    void prepareSave(const Document* doc, const fs::path& target);
    /// Writes the XML to the given path. Does not access the Document instance
    void saveTo(const fs::path& filepath, ProgressListener* listener = nullptr);
    /**
     * Writes the XML to the given stream. Does not access the Document instance with the following exception:
     * attached background images are written to disk. This is safe as long as we do not modify the background images
     * anywhere
     */
    void saveTo(OutputStream* out, const fs::path& filepath, ProgressListener* listener = nullptr);
    /// Update document information. Requires write access to the Document.
    void updateDocumentInfo(Document* doc);

    const std::string& getErrorMessage();

protected:
    static std::string getColorStr(Color c, unsigned char alpha = 0xff);

    virtual void visitPage(XmlNode* root, ConstPageRef p, const Document* doc, int id, const fs::path& target);
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

    struct ImageInfo {
        BackgroundImage image;  ///< This is a wrapped shared pointer
        std::optional<fs::path> newPath;
        int newId;
    };
    std::vector<ImageInfo> backgroundImages{};
};
