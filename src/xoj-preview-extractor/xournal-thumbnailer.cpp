/*
 * Xournal++
 *
 * This small programm extracts a preview out of a xournal file
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include "../util/XojPreviewExtractor.h"

#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("xoj-preview-extractor: call with INPUT.xoj OUTPUT.png\n");
		return 1;
	}

	XojPreviewExtractor extractor;
	PreviewExtractResult result = extractor.readFile(argv[1]);

	switch (result)
	{
	case PREVIEW_RESULT_IMAGE_READ:
		// continue to write preview
		break;

	case PREVIEW_RESULT_COULD_NOT_OPEN_FILE:
		printf("xoj-preview-extractor: open input failed \"%s\"\n", argv[2]);
		return 2;

	case PREVIEW_RESULT_NO_PREVIEW:
		printf("xoj-preview-extractor: the file \"%s\" file contains no preview\n", argv[2]);
		return 5;

	case PREVIEW_RESULT_ERROR_READING_PREVIEW:
	default:
		printf("xoj-preview-extractor: no preview found, may an invalid file?\n");
		return 10;
	}


	FILE* fp = fopen(argv[2], "wb");
	if (!fp)
	{
		printf("xoj-preview-extractor: open output file \"%s\" failed!\n", argv[2]);
		return 3;
	}

	fwrite(extractor.getData(), 1, extractor.getDataLength(), fp);

	fclose(fp);

	printf("xoj-preview-extractor: successfully extracted\n");
	return 0;
}
