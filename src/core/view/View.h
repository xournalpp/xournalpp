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

#include <gtk/gtk.h>

namespace xoj {
namespace view {

enum NonAudioTreatment : bool { FADE_OUT_NON_AUDIO_ = true, NORMAL_NON_AUDIO = false };
enum EditionTreatment : bool { SHOW_CURRENT_EDITING = true, HIDE_CURRENT_EDITING = false };
enum ColorTreatment : bool { COLORBLIND = true, NORMAL_COLOR = false };

struct Context {
    cairo_t* cr;
    NonAudioTreatment fadeOutNonAudio;
    EditionTreatment showCurrentEdition;
    ColorTreatment noColor;
};

class ElementView {
public:
    virtual ~ElementView() = default;
    virtual void draw(const Context& ctx) const = 0;
};

class TexImageView;
class ImageView;
class TextView;

constexpr double OPACITY_NO_AUDIO = 0.3;
};  // namespace view
};  // namespace xoj
