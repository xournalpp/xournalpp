/*
 * Xournal++
 *
 * A job which saves a Document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <string>  // for string
#include "filesystem.h"  // for path
#include "BlockingJob.h"  // for BlockingJob
#include "control/xojfile/SaveHandler.h"  // for SaveHandler

class Control;
class SaveHandler;

class SaveJob: public BlockingJob {
public:
    SaveJob(Control* control, std::function<void(bool)> = [](bool) {});

protected:
    ~SaveJob() override;

public:
    void run() override;

    bool save();

    static void updatePreview(Control* control);

protected:
    void afterRun() override;

private:

    /**
    * @brief Attempt to create a backup file (e.g. file.xopp~).
    * @param target The path to the file to backup.
    * @return true on success, false otherwise (sets lastError).
    */
    auto createBackup(const fs::path& destination) -> bool;

    /**
    * @brief Read the original XML, detect whether it is in "legacy" format and
    * sets the StringUtils::isLegacy global flag.
    * @param target The path to the original .xopp file.
    * @param tempDir A temporary directory for extraction.
    * @param[out] originalXMLStr String to store the original extracted XML.
    * @return true on success, false otherwise (sets lastError).
    */
    auto detectLegacyFormat(const fs::path& target, const fs::path& tempDir, std::string& originalXMLStr) -> bool;

    /**
    * @brief Handles saving in "legacy" format (saves the entire document).
    * @param h The prepared SaveHandler.
    * @param target The final destination path.
    * @param[in,out] totalTime Reference to the total time counter.
    */
    auto saveLegacy(SaveHandler& h, const fs::path& target, long& totalTime) -> void;

    /**
    * @brief Handles saving in "modern" format (partial save + merge).
    * @param h The prepared SaveHandler.
    * @param target The final destination path.
    * @param originalXMLStr The XML of the original file.
    * @param tempDir The temporary directory.
    * @param[in,out] totalTime Reference to the total time counter.
    */
    auto saveModern(SaveHandler& h, const fs::path& target, const std::string& originalXMLStr, const fs::path& tempDir, long& totalTime) -> void;

    /**
    * @brief Merges the modified XML (modified pages only) with the original XML
    * to create the final file, respecting the order of the pages of the Document.
    * @param modificationXMLStr XML string containing *only* the modified pages.
    * @param originalXMLStr XML string of the complete *original* file.
    * @param target The path to the final file to save.
    */
    void saveFinalFile(Control* control, const std::string& modifiedXMLStr, const std::string& originalXMLStr, const fs::path& target, auto& lastError);

private:
    std::string lastError;
    /// Called after saving, with boolean parameter true on success, false on failure (error)
    std::function<void(bool)> callback;
};
