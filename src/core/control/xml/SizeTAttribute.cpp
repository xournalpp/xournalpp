#include "SizeTAttribute.h"

#include <string>  // for allocator, string

#include <glib.h>  // for g_free, g_strdup_printf

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream

SizeTAttribute::SizeTAttribute(const char* name, size_t value): XMLAttribute(name) { this->value = value; }

SizeTAttribute::~SizeTAttribute() = default;

void SizeTAttribute::writeOut(OutputStream* out) {
    char* str = g_strdup_printf("%zu", value);
    out->write(str);
    g_free(str);
}
