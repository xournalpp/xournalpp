/*
 * Xournal++
 *
 * Namespace for view related classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>

class Element;

namespace xoj {
namespace view {

enum NonAudioTreatment : bool { FADE_OUT_NON_AUDIO_ = true, NORMAL_NON_AUDIO = false };
enum EditionTreatment : bool { SHOW_CURRENT_EDITING = true, HIDE_CURRENT_EDITING = false };
enum ColorTreatment : bool { COLORBLIND = true, NORMAL_COLOR = false };

class Context {
public:
    cairo_t* cr;
    NonAudioTreatment fadeOutNonAudio;
    EditionTreatment showCurrentEdition;
    ColorTreatment noColor;

    static Context createDefault(cairo_t* cr) { return {cr, NORMAL_NON_AUDIO, HIDE_CURRENT_EDITING, NORMAL_COLOR}; }
    static Context createColorBlind(cairo_t* cr) { return {cr, NORMAL_NON_AUDIO, HIDE_CURRENT_EDITING, COLORBLIND}; }
};

class ElementView {
public:
    virtual ~ElementView() = default;
    virtual void draw(const Context& ctx) const = 0;
    static std::unique_ptr<ElementView> createFromElement(const Element* e);
};

class TexImageView;
class ImageView;
class StrokeView;
class TextView;

constexpr double OPACITY_NO_AUDIO = 0.3;
};  // namespace view
};  // namespace xoj
