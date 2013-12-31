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

/**
 * Build this File with: gcc xournal-thumbnailer.c -lz -o xoj-preview-extractor
 */

#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

const char BASE64_TABLE[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
                                 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                                 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1
                               };

int main(int argc, char* argv[])
{
	gzFile f;
	FILE* fp;
	int count;
	unsigned char buffer[512];

	int i;
	int x;
	int pa;
	int pr;
	const char* TAG_PAGE = "<page>";
	const char* TAG_PREVIEW = "<preview>";

	int state;
	int bufPos;
	char inBuffer[4];

	fp = NULL;

	if (argc != 3)
	{
		printf("xoj-preview-extractor: call with INPUT.xoj OUTPUT.png\n");
		return 1;
	}

	f = gzopen(argv[1], "r");

	if (f == NULL)
	{
		printf("xoj-preview-extractor: open input failed \"%s\"\n", argv[1]);
		return 2;
	}

	pa = 0;
	pr = 0;
	i = 0;
	state = 0;
	bufPos = 0;

	do
	{
		count = gzread(f, buffer, sizeof(buffer));

		i = 0;

		if (state == 0)
		{
			for (; i < count; i++)
			{
				if (TAG_PREVIEW[pr] == buffer[i])
				{
					pr++;
					if (TAG_PREVIEW[pr] == 0)
					{
						// now we are in preview data
						state = 1;
						i++;
						fp = fopen(argv[2], "wb");
						if (!fp)
						{
							printf("xoj-preview-extractor: open output file \"%s\" failed!\n", argv[2]);
							return 3;
						}
						break;
					}
				}
				else
				{
					pr = 0;
				}

				if (TAG_PAGE[pa] == buffer[i])
				{
					pa++;
					if (TAG_PAGE[pa] == 0)
					{
						printf("xoj-preview-extractor: this file contains no preview\n", argv[2]);
						return 5;
					}
				}
				else
				{
					pa = 0;
				}
			}
		}

		for (; i < count; i++)
		{
			if (buffer[i] == '<')
			{
				fclose(fp);
				printf("xoj-preview-extractor: successfully extracted\n");
				return 0;
			}

			inBuffer[bufPos++] = buffer[i];
			if (bufPos == 4)
			{
				for (x = 0; x < 4; x++)
				{
					inBuffer[x] = BASE64_TABLE[inBuffer[x]];
				}

				fputc((char) ((inBuffer[0] << 2) + ((inBuffer[1] & 0x30) >> 4)), fp);
				fputc((char) (((inBuffer[1] & 0xf) << 4) + ((inBuffer[2] & 0x3c) >> 2)), fp);
				fputc((char) (((inBuffer[2] & 0x3) << 6) + inBuffer[3]), fp);

				bufPos = 0;
			}
		}

	}
	while (count);

	gzclose(f);

	printf("xoj-preview-extractor: no preview found, may an invalid file?\n");
	return 10;
}
