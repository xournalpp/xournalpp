/*
 * Xournal++
 *
 * Oversees the file opening process
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <memory>  // for unique_ptr
#include <optional>

#include "filesystem.h"  // for path

class Document;
class Control;
class UndoAction;
class Settings;
class PageTemplateSettings;

class FileOpener final {
public:
    FileOpener(Control* ctrl);
    ~FileOpener();

    struct PdfInfo {
        bool isPdfAttached;
        fs::path filename;
    };

    /// Prompt the user that the PDF background is missing and offer solution options
    void promptMissingPdf(PdfInfo& missingPdf, const fs::path& filepath);

    /// Handle the response from the missing PDF dialog
    void missingPdfDialogResponseHandler(fs::path proposedPdfFilepath, int responseId);


    /**
     * (Potentially asynchronously) Opens the given file.
     */
    void openFile(fs::path filepath, bool attachToDocument, int scrollToPage);

    /**
     * Asynchronously opens the provided path and parse it as a .xopp or .xoj file. Then forcibly replaces the currently
     * opened document with the new one. Asks the user in case of doubts (e.g. wrong file version) and aborts if the
     * document was not correctly and entirely parsed.
     *
     * if isEmergencyFile == true, the file is then deleted and the document marked as unsaved
     */
    void openXoppFile(fs::path filepath, int scrollToPage, bool isEmergencyFile = false);

    /**
     * Opens the provided path and parse it as a PDF file. Then forcibly replaces the currently opened document with a
     * new one based on the PDF. WARNING: Be sure the active document has been saved (or discarded) before calling
     * openPdfFile()
     *
     * @return true on success
     */
    bool openPdfFile(fs::path filepath, bool attachToDocument, int scrollToPage);

    /**
     * Opens the provided path and parse it as a PNG file. Then forcibly replaces the currently opened document with a
     * new one based on the PNG. WARNING: Be sure the active document has been saved (or discarded) before calling
     * openPngFile()
     *
     * @return true on success
     */
    bool openPngFile(fs::path filepath, bool attachToDocument, int scrollToPage);

    /**
     * Opens the provided path and parse it as a .xopt template  file. Then forcibly replaces the currently opened
     * document with a new one based on the template. WARNING: Be sure the active document has been saved (or discarded)
     * before calling openXoptFile()
     *
     * @return true on success
     */
    bool openXoptFile(fs::path filepath);

    /// Creates a new document, with a single page, following the template (if any)
    static std::unique_ptr<Document> createNewDocument(Settings* settings, fs::path filepath,
                                                       const std::optional<PageTemplateSettings>& pageTemplate);

private:
    /// The document is fully loaded: asks the Control instance to take over
    void finished();

private:
    int scrollToPage = -1;
    bool isEmergencyFile = false;
    std::unique_ptr<Document> doc;
    Control* ctrl;
    std::unique_ptr<UndoAction> recoveryAction;  ///< (Maybe nullptr) Undo a recovery step taken in the loading process
};
