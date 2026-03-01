/*
 * Xournal++
 *
 * Enum and constexpr names for the tags used in .xoj and .xopp files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string_view>

#include "util/EnumIndexedArray.h"

namespace xoj::xml_tags {

enum class Type : size_t {
    UNKNOWN,
    XOURNAL,
    MRWRITER,
    TITLE,
    PREVIEW,
    PAGE,
    AUDIO,
    BACKGROUND,
    LAYER,
    TIMESTAMP,
    STROKE,
    TEXT,
    IMAGE,
    TEXIMAGE,
    ATTACHMENT,

    // This must be the last value
    ENUMERATOR_COUNT
};

// Names corresponding to the xoj::xml_tags::Type enum. They must imperatively correspond to the order of the enum!
constexpr EnumIndexedArray<std::u8string_view, Type> NAMES = {
        u8"[unknown]", u8"xournal",   u8"MrWriter", u8"title", u8"preview", u8"page",     u8"audio",     u8"background",
        u8"layer",     u8"timestamp", u8"stroke",   u8"text",  u8"image",   u8"teximage", u8"attachment"};

}  // namespace xoj::xml_tags
