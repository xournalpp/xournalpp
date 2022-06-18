#include "IntAttribute.h"

#include <string>  // for allocator, string

#include <glib.h>  // for g_free, g_strdup_printf

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream

IntAttribute::IntAttribute(const char* name, int value): XMLAttribute(name) { this->value = value; }

IntAttribute::~IntAttribute() = default;

void IntAttribute::writeOut(OutputStream* out) {
    char* str = g_strdup_printf("%i", value);
    out->write(str);
    g_free(str);
}
