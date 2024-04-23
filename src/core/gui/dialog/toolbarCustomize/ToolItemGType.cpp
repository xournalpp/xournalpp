#include "ToolItemGType.h"

#include "util/Assert.h"

namespace xoj::dnd {
/**
 * This GType is the same as G_TYPE_STRING except it is not convertible to strings.
 * This is used to forbid internal drag'n'drop operations from being exposed to other apps
 */
GType get_tool_item_gtype() {
    static GType XOPP_TYPE_TOOL_ITEM = 0;
    if (XOPP_TYPE_TOOL_ITEM == 0) {
        XOPP_TYPE_TOOL_ITEM = g_type_fundamental_next();
        const GTypeFundamentalInfo finfo = {};
        GTypeInfo info = {
                0,    /* class_size */
                NULL, /* base_init */
                NULL, /* base_destroy */
                NULL, /* class_init */
                NULL, /* class_destroy */
                NULL, /* class_data */
                0,    /* instance_size */
                0,    /* n_preallocs */
                NULL, /* instance_init */
                NULL, /* value_table */
        };
        info.value_table = g_type_value_table_peek(G_TYPE_STRING);
        GType type = g_type_register_fundamental(XOPP_TYPE_TOOL_ITEM, "xopp-tool-item", &info, &finfo, (GTypeFlags)0);
        g_assert(type == XOPP_TYPE_TOOL_ITEM);
    }
    return XOPP_TYPE_TOOL_ITEM;
}

const char* g_value_get_tool_item_string(GValue* value) {
    xoj_assert((G_TYPE_CHECK_VALUE_TYPE((value), get_tool_item_gtype())));
    return static_cast<const char*>(value->data[0].v_pointer);
}
}  // namespace xoj::dnd
