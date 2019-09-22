#include "HexObjectEncoding.h"

#include <stdio.h>

HexObjectEncoding::HexObjectEncoding()
{
}

HexObjectEncoding::~HexObjectEncoding()
{
}

void HexObjectEncoding::addData(const void* data, int len)
{
	char* buffer = (char*) g_malloc(len * 2);

	for (int i = 0; i < len; i++)
	{
		int x = ((unsigned char*) data)[i];
		sprintf(&buffer[i * 2], "%02x", x);
	}

	g_string_append_len(this->data, buffer, len * 2);

	g_free(buffer);
}
