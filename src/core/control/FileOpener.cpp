#include "FileOpener.h"

#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <utility>  // for move
#include <vector>

#include "control/settings/Settings.h"
#include "control/xojfile/LoadHandler.h"
#include "gui/XournalppCursor.h"
#include "gui/dialog/XojOpenDlg.h"
#include "model/XojPage.h"
#include "undo/EmergencySaveRestore.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"

#include "Control.h"
#include "PageBackgroundChangeController.h"
#include "config.h"
#include "filesystem.h"

FileOpener::FileOpener(Control* ctrl): ctrl(ctrl) {}

FileOpener::~FileOpener() = default;

void FileOpener::finished() {
    if (this->isEmergencyFile) {
        auto file = this->doc->getFilepath();
        std::error_code error;
        if (!fs::remove(file, error)) {
            std::stringstream msg;
            msg << FS(_F("Failed to delete file: {1}") % file.u8string()) << std::endl;
            msg << error << std::endl << error.message() << std::endl;
            msg << FS(_F("Please delete the file manually"));
            XojMsgBox::showErrorToUser(ctrl->getGtkWindow(), msg.str());
        }

        this->doc->setFilepath("");
        if (!this->recoveryAction) {
            // Ensures the document is marked as unsaved
            this->recoveryAction = std::make_unique<EmergencySaveRestore>();
        }
    }
    this->ctrl->finishedOpeningFileCallback(std::move(this->doc), this->scrollToPage, std::move(this->recoveryAction));
}

static auto formatErrorMessages(const std::vector<std::string>& errorMessages) -> std::string {
    // Deduplicate identical repeated messages
    std::vector<std::string> deduplicated;

    for (std::size_t i = 0; i < errorMessages.size();) {
        std::size_t j = i + 1;
        while (j < errorMessages.size() && errorMessages[j] == errorMessages[i]) {
            ++j;
        }
        const std::size_t count = j - i;

        deduplicated.emplace_back(StringUtils::markup_escape(errorMessages[i]));
        if (count == 2) {
            deduplicated.emplace_back(deduplicated.back());
        } else if (count > 2) {
            deduplicated.emplace_back(FS(_F("<i>Above error repeated {1} times</i>") % (count - 1)));
        }

        i = j;
    }

    // Remove middle messages if the list is still too long
    constexpr std::size_t MAX_MESSAGE_COUNT = 12;
    if (deduplicated.size() > MAX_MESSAGE_COUNT) {
        constexpr std::size_t BEFORE = MAX_MESSAGE_COUNT / 2;
        constexpr std::size_t AFTER = MAX_MESSAGE_COUNT - BEFORE - 1;  // reserve one line for the overflow indication
        deduplicated[BEFORE] =
                FS(_F("<i>&lt;{1} more lines of error messages&gt;</i>") % (deduplicated.size() - (BEFORE + AFTER)));
        deduplicated.erase(deduplicated.begin() + BEFORE + 1, deduplicated.end() - AFTER);
    }

    if (deduplicated.empty()) {
        return "";
    }

    // Todo(C++23): use std::views::join_with
    std::size_t totalSize = deduplicated.size() - 1;  // for '\n' characters
    for (auto& str: deduplicated) {
        totalSize += str.size();
    }
    std::string result;
    result.reserve(totalSize);
    result.append(deduplicated.front());
    for (auto it = deduplicated.begin() + 1; it != deduplicated.end(); ++it) {
        result.append('\n' + *it);
    }
    return result;
}

void FileOpener::openXoppFile(fs::path filepath, int scrollToPage, bool isEmergencyFile) {
    std::unique_ptr<Document> doc{};
    std::optional<PdfInfo> missingPdf{};
    int fileVersion{};
    std::vector<std::string> errorMessages{};

    this->scrollToPage = scrollToPage;
    this->isEmergencyFile = isEmergencyFile;

    try {
        LoadHandler loadHandler(&errorMessages);
        doc = loadHandler.loadDocument(filepath);

        if (!loadHandler.getMissingPdfFilename().empty() || loadHandler.isAttachedPdfMissing()) {
            missingPdf = {loadHandler.isAttachedPdfMissing(), loadHandler.getMissingPdfFilename()};
        }
        fileVersion = loadHandler.getFileVersion();
    } catch (std::exception& e) {
        std::string msg = FS(_F("Error opening file \"{1}\".\n\n"
                                "<tt>{2}</tt>\n<b>{3}</b>") %
                             filepath.u8string() % formatErrorMessages(errorMessages) % e.what());
        XojMsgBox::showMarkupMessageToUser(this->ctrl->getGtkWindow(), msg, "", GTK_MESSAGE_ERROR);
        this->finished();
        return;
    }

    auto afterOpen = [self = this, missingPdf = std::move(missingPdf), errorMessages = std::move(errorMessages),
                      doc = std::move(doc), filepath]() mutable {
        self->doc = std::move(doc);

        if (!errorMessages.empty()) {
            std::string msg =
                    FS(_F("There were some errors while loading the file. Some information might have been lost. "
                          "Do not overwrite your old file unless you are sure everything you need was loaded "
                          "correctly.\n\n"
                          "Error messages:\n<tt>{1}</tt>") %
                       formatErrorMessages(errorMessages));
            XojMsgBox::showMarkupMessageToUser(self->ctrl->getGtkWindow(), msg, "", GTK_MESSAGE_WARNING);
        }

        if (missingPdf && (missingPdf->isPdfAttached || !missingPdf->filename.empty())) {
            // give the user a second chance to select a new PDF filepath, or to discard the PDF
            self->promptMissingPdf(missingPdf.value(), filepath);
        } else {
            self->finished();
        }
    };

    if (fileVersion > FILE_FORMAT_VERSION) {
        enum { YES = 1, NO };
        std::vector<XojMsgBox::Button> buttons = {{_("Yes"), YES}, {_("No"), NO}};
        XojMsgBox::askQuestion(
                this->ctrl->getGtkWindow(), _("File version mismatch"),
                _("The file being loaded has a file format version newer than the one currently supported by this "
                  "version of Xournal++, so it may not load properly. Open anyways?"),
                buttons, [afterOpen = std::move(afterOpen), self = this](int response) mutable {
                    if (response == YES) {
                        afterOpen();
                    } else {
                        self->finished();
                    }
                });
    } else {
        afterOpen();
    }
}

bool FileOpener::openPdfFile(fs::path filepath, bool attachToDocument, int scrollToPage) {
    this->ctrl->getCursor()->setCursorBusy(true);
    auto doc = std::make_unique<Document>(nullptr);
    bool success = doc->readPdf(filepath, /*initPages=*/true, attachToDocument);
    if (success) {
        this->doc = std::move(doc);
        this->scrollToPage = scrollToPage;
    } else {
        std::string msg = FS(_F("Error reading PDF file \"{1}\"\n{2}") % filepath.u8string() % doc->getLastErrorMsg());
        XojMsgBox::showErrorToUser(this->ctrl->getGtkWindow(), msg);
    }
    this->ctrl->getCursor()->setCursorBusy(false);
    this->finished();
    return success;
}

static void addDefaultPage(const PageTemplateSettings& model, Document* doc) {
    auto page = std::make_shared<XojPage>(model.getPageWidth(), model.getPageHeight());
    page->setBackgroundColor(model.getBackgroundColor());
    page->setBackgroundType(model.getBackgroundType());
    doc->addPage(std::move(page));
}

std::unique_ptr<Document> FileOpener::createNewDocument(Settings* settings, fs::path filepath,
                                                        const std::optional<PageTemplateSettings>& pageTemplate) {
    auto newDoc = std::make_unique<Document>(nullptr);
    if (!filepath.empty()) {
        newDoc->setFilepath(std::move(filepath));
    }
    const auto& model = pageTemplate.value_or(settings->getPageTemplateSettings());

    addDefaultPage(model, newDoc.get());
    return newDoc;
}

bool FileOpener::openPngFile(fs::path filepath, bool attachToDocument, int scrollToPage) {
    fs::path imagePath(filepath);
    this->ctrl->getCursor()->setCursorBusy(true);
    auto doc = createNewDocument(this->ctrl->getSettings(), std::move(filepath), std::nullopt);

    // Put a png file directly in the page
    PageRef page = this->doc->getPage(0);
    BackgroundImage img;
    GError* error = nullptr;
    img.loadFile(imagePath, &error);
    img.setAttach(attachToDocument);

    if (error) {
        std::string msg = FS(_F("Error reading PNG file \"{1}\"") % imagePath.u8string());
        XojMsgBox::showErrorToUser(this->ctrl->getGtkWindow(), msg);
        g_error_free(error);
        this->ctrl->getCursor()->setCursorBusy(false);
        this->finished();
        return false;
    }

    page->setBackgroundImage(std::move(img));
    page->setBackgroundType(PageType(PageTypeFormat::Image));


    // Apply correct page size
    GdkPixbuf* pixbuf = page->getBackgroundImage().getPixbuf();
    if (pixbuf) {
        page->setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    } else {
        g_warning("PageBackgroundChangeController::setPageImageBackground(): Page with image background but nullptr "
                  "pixbuf");
    }

    this->ctrl->getCursor()->setCursorBusy(false);
    this->doc = std::move(doc);
    this->scrollToPage = scrollToPage;
    this->finished();
    return true;
}

bool FileOpener::openXoptFile(fs::path filepath) {
    auto pageTemplate = Util::readString(filepath);
    if (!pageTemplate) {
        // Unable to read the template from the file
        // Error message has already been displayed
        this->finished();
        return false;
    }

    PageTemplateSettings model;
    if (!model.parse(*pageTemplate)) {
        const auto msg = FS(_F("Error reading template file \"{1}\"") % filepath.u8string());
        XojMsgBox::showErrorToUser(this->ctrl->getGtkWindow(), msg);
        this->finished();
        return false;
    }

    this->doc = createNewDocument(this->ctrl->getSettings(), std::move(filepath), model);
    this->finished();
    return true;
}

void FileOpener::openFile(fs::path filepath, bool attachToDocument, int scrollToPage) {
    if (filepath.empty()) {
        this->doc = createNewDocument(this->ctrl->getSettings(), fs::path(), std::nullopt);
        finished();
        return;
    }

    if (std::error_code err; !fs::exists(filepath, err)) {
        std::string message =
                err ? FS(_F("Failed to determine if path exists \"{1}\": {2}") % filepath.u8string() % err.message()) :
                      FS(_F("That file does not exist:\n\"{1}\"") % filepath.u8string());
        XojMsgBox::showErrorToUser(ctrl->getGtkWindow(), message);
        finished();
        return;
    }

    if (filepath.extension() == ".xopt") {
        this->openXoptFile(std::move(filepath));
        return;
    }

    if (Util::hasPngFileExt(filepath)) {
        this->openPngFile(std::move(filepath), attachToDocument, scrollToPage);
        return;
    }

    if (Util::hasPdfFileExt(filepath)) {
        if (!attachToDocument && this->ctrl->getSettings()->isAutoloadPdfXoj()) {
            const std::vector<std::string> exts = {".xopp", ".xoj", ".pdf.xopp", ".pdf.xoj"};
            fs::path root = filepath;
            Util::clearExtensions(root, ".pdf");
            for (const std::string& ext: exts) {
                fs::path f = root;
                f += ext;
                if (fs::exists(f)) {
                    this->openXoppFile(std::move(f), scrollToPage);
                    return;
                }
            }
        }
        this->openPdfFile(std::move(filepath), attachToDocument, scrollToPage);
        return;
    }

    this->openXoppFile(std::move(filepath), scrollToPage);
}
enum class MissingPdfDialogOptions : gint { USE_PROPOSED, SELECT_OTHER, REMOVE, CANCEL };

void FileOpener::promptMissingPdf(PdfInfo& missingPdf, const fs::path& filepath) {
    const fs::path& missingFilePath = missingPdf.filename;

    // create error message
    std::string parentFolderPath;
    std::string filename;
#if defined(WIN32)
    parentFolderPath = missingFilePath.parent_path().string();
    filename = missingFilePath.filename().string();
#else
    // since POSIX systems detect the whole Windows path as a filename, this checks whether missingFilePath
    // contains a Windows path
    std::regex regex(R"([A-Z]:\\(?:.*\\)*(.*))");
    std::cmatch matchInfo;

    if (std::regex_match(missingFilePath.filename().string().c_str(), matchInfo, regex) && matchInfo[1].matched) {
        parentFolderPath = missingFilePath.filename().string();
        filename = matchInfo[1].str();
    } else {
        parentFolderPath = missingFilePath.parent_path().string();
        filename = missingFilePath.filename().string();
    }
#endif
    std::string msg;
    if (missingPdf.isPdfAttached) {
        msg = FS(_F("The attached background file could not be found. It might have been moved, "
                    "renamed or deleted."));
    } else {
        msg = FS(_F("The background file \"{1}\" could not be found. It might have been moved, renamed or "
                    "deleted.\nIt was last seen at: \"{2}\"") %
                 filename % parentFolderPath);
    }

    // try to find file in current directory
    auto proposedPdfFilepath = filepath.parent_path() / filename;
    bool proposePdfFile = !missingPdf.isPdfAttached && !filename.empty() && fs::exists(proposedPdfFilepath) &&
                          !fs::is_directory(proposedPdfFilepath);
    if (proposePdfFile) {
        msg += FS(_F("\nProposed replacement file: \"{1}\"") % proposedPdfFilepath.u8string());
    }

    // show the dialog
    std::vector<XojMsgBox::Button> buttons = {
            {_("Select another PDF"), static_cast<int>(MissingPdfDialogOptions::SELECT_OTHER)},
            {_("Remove PDF Background"), static_cast<int>(MissingPdfDialogOptions::REMOVE)},
            {_("Cancel"), static_cast<int>(MissingPdfDialogOptions::CANCEL)}};
    if (proposePdfFile) {
        buttons.insert(buttons.begin(),
                       {_("Use proposed PDF"), static_cast<int>(MissingPdfDialogOptions::USE_PROPOSED)});
    }
    XojMsgBox::askQuestion(
            this->ctrl->getGtkWindow(), _("Missing PDF background file"), msg, buttons,
            std::bind(&FileOpener::missingPdfDialogResponseHandler, this, proposedPdfFilepath, std::placeholders::_1));
}

void FileOpener::missingPdfDialogResponseHandler(fs::path proposedPdfFilepath, int responseId) {
    xoj_assert(this->doc);
    switch (static_cast<MissingPdfDialogOptions>(responseId)) {
        case MissingPdfDialogOptions::USE_PROPOSED:
            if (!proposedPdfFilepath.empty()) {
                this->recoveryAction = PageBackgroundChangeController::changeDocumentPdfBackground(
                        this->doc.get(), proposedPdfFilepath, false, this->ctrl);
            }
            break;
        case MissingPdfDialogOptions::SELECT_OTHER:
            Util::execInUiThread([this]() {
                xoj::OpenDlg::showAnnotatePdfDialog(
                        ctrl->getGtkWindow(), ctrl->getSettings(), [this](fs::path path, bool attachPdf) {
                            if (!path.empty()) {
                                this->recoveryAction = PageBackgroundChangeController::changeDocumentPdfBackground(
                                        this->doc.get(), path, attachPdf, this->ctrl);
                            }
                            finished();  // TODO fix: callback is not called if user cancels
                        });
            });
            return;  // Do not call finished() yet: wait for the dialog to be closed
        case MissingPdfDialogOptions::REMOVE:
            this->recoveryAction =
                    PageBackgroundChangeController::changeDocumentPdfBackground(this->doc.get(), "", false, this->ctrl);
            break;
        default:
            break;
    }
    finished();
}
