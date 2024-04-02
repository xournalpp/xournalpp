/*
 * Xournal++
 *
 * A TexImage on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>  // for string

#include <cairo.h>    // for cairo_surface_t, cairo_status_t
#include <glib.h>     // for GError
#include <poppler.h>  // for PopplerDocument

#include "util/Range.h"
#include "util/raii/GObjectSPtr.h"  // for GObjectSPtr

#include "Element.h"  // for Element

class ObjectInputStream;
class ObjectOutputStream;


class TexImage: public Element {
public:
    TexImage();
    TexImage(const TexImage&) = delete;
    TexImage(const TexImage&&) = delete;
    auto operator=(const TexImage&) -> TexImage& = delete;
    auto operator=(const TexImage&&) -> TexImage&& = delete;
    ~TexImage() override;

public:
    /**
     * Returns the binary data (PDF or PNG (deprecated)).
     */
    auto getBinaryData() const -> const std::string&;

    /**
     * @return The PDF Document, if rendered as a PDF.
     *
     * The document needs to be referenced, if it will be hold somewhere
     */
    auto getPdf() const -> PopplerDocument*;

    // text tag to alow latex
    void setText(std::string text);
    auto getText() const -> std::string;

    auto cloneTexImage() const -> std::unique_ptr<TexImage>;
    auto clone() const -> ElementPtr override;

    /**
     * @return true if the binary data (PNG or PDF) was loaded successfully.
     */
    auto loadData(std::string&& bytes, GError** err = nullptr) -> bool;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    auto internalUpdateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> override;

    /**
     * Free image and PDF
     */
    void freeImageAndPdf();

private:
    // cache:
    xoj::util::Rectangle<double> bounds{};
    std::string binaryData;  ///< PDF Documents binary data, must stay alive as long as the PDF is used
    xoj::util::GObjectSPtr<PopplerDocument> pdf;  ///<

    // data:
    std::string text;  ///< tex expression
};
