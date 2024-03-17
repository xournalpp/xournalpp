/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>   // for size_t
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string, allocator
#include <vector>    // for vector

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>                // for GApplication
#include <glib.h>                   // for guint
#include <gtk/gtk.h>                // for GtkLabel

#include "control/ToolEnums.h"               // for ToolSize, ToolType
#include "control/jobs/ProgressListener.h"   // for ProgressListener
#include "control/settings/SettingsEnums.h"  // for LayoutType
#include "control/settings/ViewModes.h"      // for ViewModeId
#include "control/tools/EditSelection.h"     // for OrderChange
#include "enums/Action.enum.h"               // for Action
#include "model/DocumentHandler.h"           // for DocumentHandler
#include "model/DocumentListener.h"          // for DocumentListener
#include "model/GeometryTool.h"              // for GeometryTool
#include "model/PageRef.h"                   // for PageRef
#include "undo/UndoRedoHandler.h"            // for UndoRedoHandler (ptr only)

#include "ClipboardHandler.h"  // for ClipboardListener
#include "ToolHandler.h"       // for ToolListener
#include "filesystem.h"        // for path

class LoadHandler;
class GeometryToolController;
class AudioController;
class FullscreenHandler;
class Sidebar;
class GladeSearchpath;
class MetadataManager;
class XournalppCursor;
class ToolbarDragDropHandler;
class MetadataEntry;
class MetadataCallbackData;
class PageBackgroundChangeController;
class PageTypeHandler;
class BaseExportJob;
class LayerController;
class PluginController;
class Document;
class EditSelection;
class Element;
class MainWindow;
class ObjectInputStream;
class ScrollHandler;
class SearchBar;
class Settings;
class TextEditor;
class XournalScheduler;
class ZoomControl;
class ToolMenuHandler;
class XojFont;
class XojPdfRectangle;
class Callback;
class ActionDatabase;

class Control:
        public ToolListener,
        public DocumentHandler,
        public UndoRedoListener,
        public ClipboardListener,
        public ProgressListener {
public:
    Control(GApplication* gtkApp, GladeSearchpath* gladeSearchPath, bool disableAudio);
    Control(Control const&) = delete;
    Control(Control&&) = delete;
    auto operator=(Control const&) -> Control& = delete;
    auto operator=(Control&&) -> Control& = delete;
    ~Control() override;

    void initWindow(MainWindow* win);

public:
    // Menu File
    bool newFile(std::string pageTemplate = "", fs::path filepath = {});
    bool openFile(fs::path filepath = "", int scrollToPage = -1, bool forceOpen = false);
    bool annotatePdf(fs::path filepath, bool attachToDocument);
    void print();
    void exportAsPdf();
    void exportAs();
    void quit(bool allowCancel = true);

    /**
     * Save the current document.
     *
     * @param synchron Whether the save should be run synchronously or asynchronously.
     */
    bool save(bool synchron = false);
    bool saveAs();

    /**
     * Marks the current document as saved if it is currently marked as unsaved.
     */
    void resetSavedStatus();

    /**
     * Close the current document, prompting to save unsaved changes.
     *
     * @param allowDestroy Whether clicking "Discard" should destroy the current document.
     * @param allowCancel Whether the user should be able to cancel closing the document.
     * @return true if the user closed the document, otherwise false.
     */
    bool close(bool allowDestroy = false, bool allowCancel = true);

    // Asks user to replace an existing file when saving / exporting, since we add the extension
    // after the OK, we need to check manually
    bool askToReplace(fs::path const& filepath) const;

    // Menu edit
    void showSettings();

    // The core handler for inserting latex
    void runLatex();

    // Menu Help
    void showAbout();

    /**
     * @brief Update the Cursor and the Toolbar based on the active color
     *
     */
    void toolColorChanged() override;
    /**
     * @brief Change the color of the current selection based on the active Tool
     *
     */
    void changeColorOfSelection() override;
    void toolChanged() override;
    void toolSizeChanged() override;
    void toolFillChanged() override;
    void toolLineStyleChanged() override;

    void selectTool(ToolType type);
    void selectDefaultTool();

    void setFontSelected(const XojFont& font);  ///< Modifies the Action state without triggering callbacks
    void fontChanged(const XojFont& font);      ///< Set the font after the user selected a font

    void updatePageNumbers(size_t page, size_t pdfPage);

    /**
     * Save current state (selected tool etc.)
     */
    void saveSettings();

    void updateWindowTitle();
    void setViewPairedPages(bool enabled);
    void setViewFullscreenMode(bool enabled);
    void setViewPresentationMode(bool enabled);
    void setPairsOffset(int numOffset);
    void setViewColumns(int numColumns);
    void setViewRows(int numRows);
    void setViewLayoutType(LayoutType type);
    void setViewLayoutVert(bool vert);
    void setViewLayoutR2L(bool r2l);
    void setViewLayoutB2T(bool b2t);

    void manageToolbars();
    void customizeToolbars();
    void setFullscreen(bool enabled);
    void setShowSidebar(bool enabled);
    void setShowToolbar(bool enabled);
    void setShowMenubar(bool enabled);

    void gotoPage();

    void setToolDrawingType(DrawingType type);

    void paperTemplate();
    void paperFormat();
    void changePageBackgroundColor();
    void updateBackgroundSizeButton();

    /**
     * Loads the view mode (hide/show menu-,tool-&sidebar)
     */
    bool loadViewMode(ViewModeId mode);

    /**
     * @brief Search text on the given page. The matches (if any) are stored in the XojPageView::SearchControl instance.
     * @param occurrences If not nullptr, the pointed variable will contain the number of matches on the page
     * @param matchRect If not nullptr, will contain the topleft point of the first match on the page
     *                          (Used for scrolling to the first match)
     * @return true if at least one match was found
     */
    bool searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                          XojPdfRectangle* matchRect);

    /**
     * Fire page selected, but first check if the page Number is valid
     *
     * @return the page ID or size_t_npos if the page is not found
     */
    size_t firePageSelected(const PageRef& page);
    void firePageSelected(size_t page);

    void addDefaultPage(std::string pageTemplate);
    void duplicatePage();
    void insertNewPage(size_t position, bool shouldScrollToPage = true);
    void appendNewPdfPages();
    void insertPage(const PageRef& page, size_t position, bool shouldScrollToPage = true);
    void deletePage();

    /**
     * Disable / enable delete page button
     */
    void updateDeletePageButton();

    // selection handling
    void clearSelection();

    void moveSelectionToLayer(size_t layerNo);

    void setCopyCutEnabled(bool enabled);

    void enableAutosave(bool enable);

    void clearSelectionEndText();

    void selectAllOnPage();

    void reorderSelection(EditSelection::OrderChange change);

    void setToolSize(ToolSize size);

    /**
     * Change the line style of the PEN if select, or of selected elements if any
     * Otherwise, select the PEN tool and set its linestyle
     */
    void setLineStyle(const std::string& style);

    /**
     * Change the eraser type. If the eraser is not selected, select it as well.
     */
    void setEraserType(EraserType type);

    void setFill(bool fill);

    TextEditor* getTextEditor();

    GladeSearchpath* getGladeSearchPath() const;

    void disableSidebarTmp(bool disabled);

    XournalScheduler* getScheduler() const;

    void block(const std::string& name);
    void unblock();

    void setLastAutosaveFile(fs::path newAutosaveFile);
    void deleteLastAutosaveFile();
    void setClipboardHandlerSelection(EditSelection* selection);

    void addChangedDocumentListener(DocumentListener* dl);
    void removeChangedDocumentListener(DocumentListener* dl);

    MetadataManager* getMetadataManager() const;
    Settings* getSettings() const;
    ToolHandler* getToolHandler() const;
    ZoomControl* getZoomControl() const;
    Document* getDocument() const;
    UndoRedoHandler* getUndoRedoHandler() const;
    MainWindow* getWindow() const;
    GtkWindow* getGtkWindow() const;
    ScrollHandler* getScrollHandler() const;
    PageRef getCurrentPage();
    size_t getCurrentPageNo() const;
    XournalppCursor* getCursor() const;
    Sidebar* getSidebar() const;
    SearchBar* getSearchBar() const;
    AudioController* getAudioController() const;
    PageTypeHandler* getPageTypes() const;
    PageBackgroundChangeController* getPageBackgroundChangeController() const;
    LayerController* getLayerController() const;
    PluginController* getPluginController() const;


    bool copy();
    bool cut();
    bool paste();

    void help();

    void selectAlpha(OpacityFeature feature);

    /**
     * @brief Initialize the all button tools based on the respective ButtonConfigs
     *
     */
    void initButtonTool();


public:
    // UndoRedoListener interface
    void undoRedoChanged() override;
    void undoRedoPageChanged(PageRef page) override;

public:
    // ProgressListener interface
    void setMaximumState(size_t max) override;
    void setCurrentState(size_t state) override;

public:
    // ClipboardListener interface
    void clipboardCutCopyEnabled(bool enabled) override;
    void clipboardPasteEnabled(bool enabled) override;
    void clipboardPasteText(std::string text) override;
    void clipboardPasteImage(GdkPixbuf* img) override;
    void clipboardPasteXournal(ObjectInputStream& in) override;
    void deleteSelection() override;

    void clipboardPaste(ElementPtr e);

public:
    void registerPluginToolButtons(ToolMenuHandler* toolMenuHandler);
    inline ActionDatabase* getActionDatabase() const { return actionDB.get(); }

protected:
    void setRotationSnapping(bool enable);
    void setGridSnapping(bool enable);

    bool showSaveDialog();
    void showFontDialog();
    void showColorChooserDialog();

    void fileLoaded(int scrollToPage = -1);

    void eraserSizeChanged();
    void penSizeChanged();
    void highlighterSizeChanged();

    static bool checkChangedDocument(Control* control);
    static bool autosaveCallback(Control* control);

    /**
     * Load metadata later, md will be deleted
     */
    void loadMetadata(MetadataEntry md);

    static bool loadMetadataCallback(MetadataCallbackData* data);

    /**
     * Check if this is an autosave file, return false in this case and display a user instruction
     */
    bool shouldFileOpen(fs::path const& filepath) const;

    bool loadXoptTemplate(fs::path const& filepath);
    bool loadPdf(fs::path const& filepath, int scrollToPage);

private:
    /**
     * @brief Creates the specified geometric tool if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a geometric tool was created
     */
    template <class ToolClass, class ViewClass, class ControllerClass, class InputHandlerClass,
              GeometryToolType toolType>
    bool toggleGeometryTool();
    void resetGeometryTool();

    /**
     * @brief Creates a compass if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a compass was created
     */
    bool toggleCompass();
    /**
     * @brief Creates a setsquare if it's not on the current page yet. Deletes it if it already exists.
     * @return true if a setsquare was created
     */
    bool toggleSetsquare();

    /**
     * Prompt the user that the PDF background is missing and offer solution options
     */
    void promptMissingPdf(LoadHandler& loadHandler, const fs::path& filepath);

    /**
     * Handle the response from the missing PDF dialog
     */
    void missingPdfDialogResponseHandler(const fs::path& proposedPdfFilepath, int responseId);

    /**
     * "Closes" the document, preparing the editor for a new document.
     */
    void closeDocument();

    /**
     * @brief Get the pen line style to select in the toolbar
     *
     * @return style to select, empty if no style should be selected (active
     * selection with differing line styles)
     */
    auto getLineStyleToSelect() -> std::optional<std::string> const;

    UndoRedoHandler* undoRedo = nullptr;
    ZoomControl* zoom = nullptr;

    Settings* settings = nullptr;
    MainWindow* win = nullptr;

    Document* doc = nullptr;

    Sidebar* sidebar = nullptr;
    SearchBar* searchBar = nullptr;

    ToolHandler* toolHandler;

    ScrollHandler* scrollHandler;

    AudioController* audioController;

    ToolbarDragDropHandler* dragDropHandler = nullptr;

    GApplication* gtkApp = nullptr;

    /**
     * The cursor handler
     */
    XournalppCursor* cursor;

    /**
     * Timeout id: the timeout watches the changes and actualizes the previews from time to time
     */
    guint changeTimout;

    /**
     * The pages wihch has changed since the last update (for preview update)
     */
    std::vector<PageRef> changedPages;

    /**
     * DocumentListener instances that are to be updated by checkChangedDocument.
     */
    std::list<DocumentListener*> changedDocumentListeners;

    /**
     * Our clipboard abstraction
     */
    ClipboardHandler* clipboardHandler = nullptr;

    /**
     * The autosave handler ID
     */
    guint autosaveTimeout = 0;
    fs::path lastAutosaveFilename;

    XournalScheduler* scheduler;

    /**
     * State / Blocking attributes
     */
    GtkWidget* statusbar = nullptr;
    GtkLabel* lbState = nullptr;
    GtkProgressBar* pgState = nullptr;
    size_t maxState = 0;
    bool isBlocking;

    GladeSearchpath* gladeSearchPath;

    MetadataManager* metadata;

    PageTypeHandler* pageTypes;

    std::unique_ptr<PageBackgroundChangeController> pageBackgroundChangeController;

    LayerController* layerController;

    std::unique_ptr<GeometryTool> geometryTool;
    std::unique_ptr<GeometryToolController> geometryToolController;

    /**
     * Manage all Xournal++ plugins
     */
    PluginController* pluginController;

    std::unique_ptr<ActionDatabase> actionDB;
    template <Action a>
    friend struct ActionProperties;
};
