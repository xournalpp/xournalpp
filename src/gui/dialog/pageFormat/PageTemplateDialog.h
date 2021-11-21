/*
 * Xournal++
 *
 * Dialog to configure page templates
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include "control/pagetype/PageTypeMenu.h"
#include "control/settings/PageTemplateSettings.h"
#include "gui/GladeGui.h"

class PageTypeHandler;
class PageTemplateSettings;
class Settings;

class PageTemplateDialog: public GladeGui, public PageTypeMenuChangeListener {
public:
    PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types);
    virtual ~PageTemplateDialog();

public:
    virtual void show(GtkWindow* parent) override;
    virtual void changeCurrentPageBackground(PageTypeInfo* info) override;

    /**
     * The dialog was confirmed / saved
     */
    bool isSaved() const;

protected:
    /**
     * Set the message displayed to the user describing the
     * effects of the settings.
     */
    void setActionDescription(const std::string&);

    /**
     * Some settings only apply to the creation of new pages.
     *
     * @param visible false iff these settings should be hidden.
     */
    void setNewPageOnlySettingsVisible(bool visible);

protected:
    /**
     * Callback to change the contents of the dialog.
     *
     * For example, this callback might hide or show settings that
     * only apply to creating new pages from the template.
     */
    virtual void configureUI() = 0;

    /**
     * Initialize settings in this' model.
     *
     * For example, if this dialog updates a single page, the model
     * would be updated to reflect the current state of the page.
     */
    virtual void initModel(PageTemplateSettings& model) = 0;

    /**
     * Applies the contents of the given model to this' target.
     *
     * For example, this might save and update the global page template.
     */
    virtual void save(const PageTemplateSettings& model) = 0;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
