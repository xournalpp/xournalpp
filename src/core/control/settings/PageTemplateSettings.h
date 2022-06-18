/*
 * Xournal++
 *
 * Page template settings handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/PageType.h"  // for PageType
#include "util/Color.h"      // for Color


class PageTemplateSettings {
public:
    PageTemplateSettings();
    virtual ~PageTemplateSettings();

public:
    /**
     * Parse a template string
     *
     * @return true if valid
     */
    bool parse(const std::string& tpl);

    /**
     * Convert to a parsable string
     */
    std::string toString() const;

    bool isCopyLastPageSettings() const;
    void setCopyLastPageSettings(bool copyLastPageSettings);

    bool isCopyLastPageSize() const;
    void setCopyLastPageSize(bool copyLastPageSize);

    double getPageWidth() const;
    void setPageWidth(double pageWidth);

    double getPageHeight() const;
    void setPageHeight(double pageHeight);

    Color getBackgroundColor() const;
    void setBackgroundColor(Color backgroundColor);

    PageType getBackgroundType();
    PageType getPageInsertType();
    void setBackgroundType(const PageType& backgroundType);

private:
    /**
     * Copy the settings from the last page
     */
    bool copyLastPageSettings;

    /**
     * Copy the last page size
     */
    bool copyLastPageSize;

    double pageWidth;
    double pageHeight;

    /**
     * Background color in RGB
     */
    Color backgroundColor;

    /**
     * Background type
     */
    PageType backgroundType;
};
