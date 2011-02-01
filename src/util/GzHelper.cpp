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

	uLongf len = str->len + (str->len / 1000) + 15;
	GString * dest = g_string_sized_new(len);

	int result = compress2((Bytef*) dest->str, &len, (Bytef*) str->str, str->len, level);
	if (result != Z_OK) {
		g_string_free(dest, true);
		return NULL;
	}
	dest->len = len;

	return dest;
}

GString * GzHelper::gzuncompress(GString* str) {
	return gzuncompress(str->str, str->len);
}

GString * GzHelper::gzuncompress(const char * data, gsize len) {
	int status;
	unsigned int factor = 1, maxfactor = 16;
	char *s1 = NULL, *s2 = NULL;

	gsize length = 0;
	/*
	 zlib::uncompress() wants to know the output data length
	 if none was given as a parameter
	 we try from input length * 2 up to input length * 2^15
	 doubling it whenever it wasn't big enough
	 that should be eneugh for all real life cases
	 */

	GString * str = NULL;
	do {
		length = (unsigned long) len * (1 << factor++) + 1;
		if (str) {
			g_string_free(str, true);
		}
		str = g_string_sized_new(length);
		length--;
		status = uncompress((Bytef *) str->str, (uLongf*) &length, (Bytef*) data, (uLong) len);
		str->len = length;
		str->str[length] = 0;
	} while ((status == Z_BUF_ERROR) && (factor < maxfactor));

	if (status == Z_OK) {
		return str;
	} else {
		return NULL;
	}
}

