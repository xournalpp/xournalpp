/*
 * Xournal++
 *
 * An Alignment enum class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <array>

#include <glib.h>  // for g_warning
#include <pango/pango.h>

class TextAlignment {
public:
    enum Value { LEFT, CENTER, RIGHT };
    static constexpr std::array<const char8_t*, 3> NAMES = {u8"left", u8"center", u8"right"};
    constexpr TextAlignment(Value v): value(v) {}

    // Implicit conversion to underlying enum type
    inline constexpr operator const Value&() const { return value; }
    inline constexpr operator Value&() { return value; }

    inline constexpr PangoAlignment toPango() const {
        //  Must match the enum Value
        return PANGO_ALIGNMENT[value];
    }

    inline TextAlignment& validate() {
        if (value != LEFT && value != CENTER && value != RIGHT) {
            g_warning("Correcting invalid TextAlignment value: %d", value);
            value = LEFT;
        }
        return *this;
    }

private:
    Value value = LEFT;
    static constexpr PangoAlignment PANGO_ALIGNMENT[] = {PANGO_ALIGN_LEFT, PANGO_ALIGN_CENTER, PANGO_ALIGN_RIGHT};
};
