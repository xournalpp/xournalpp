#include "GzHelper.h"

#include <zlib.h>

GzHelper::GzHelper() {
}

GzHelper::~GzHelper() {
}

GString * GzHelper::gzcompress(GString * str, int level) {
	if ((level < -1) || (level > 9)) {
		g_warning("GzHelper::gzcompress compression level (%i) must be within -1..9", level);
		level = -1;
	}

	g_return_val_if_fail(str != NULL, NULL);

	gsize len = str->len + (str->len / 1000) + 15;
	GString * dest = g_string_sized_new(len);

	int result = compress2((Bytef*)dest->str, &len, (Bytef*)str->str, str->len, level);
	if (result != Z_OK) {
		g_string_free(dest, true);
		return NULL;
	}
	dest->len = len;

	return dest;
}
