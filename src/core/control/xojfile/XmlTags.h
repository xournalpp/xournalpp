/*
 * Xournal++
 *
 * Enum and contexpr names for the tags used in .xoj and .xopp files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

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
constexpr EnumIndexedArray<const char*, Type> NAMES = {"[unknown]", "xournal", "MrWriter",   "title",    "preview",
                                                       "page",      "audio",   "background", "layer",    "timestamp",
                                                       "stroke",    "text",    "image",      "teximage", "attachment"};

}  // namespace xoj::xml_tags
