#include <Windows.h>
#include "Loaders.h"
#include "IO.h"


struct DDS_PIXELFORMAT {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDS_HEADER {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
};


DDSTexture Loaders::LoadDDSTexture(char *path)
{
	File file = ReadFile(path);
	uint8_t *fileData = (uint8_t *)file.data;
	uint32_t magicNumber = *((uint32_t *)fileData);
	fileData += sizeof(magicNumber);
	DDS_HEADER *header = (DDS_HEADER *)(fileData);
	fileData += sizeof(DDS_HEADER);
	DDSTexture result;
	result.width = header->dwWidth;
	result.height = header->dwHeight;
	result.mipCount = header->dwMipMapCount;
	result.bytesPerPixel = 4;
	uint32_t texSize = file.size - sizeof(magicNumber) - sizeof(DDS_HEADER);
	result.data = malloc(texSize);
	memcpy(result.data, fileData, texSize);
	ReleaseFile(file);
	return result;
}

int StringToInt(char *str)
{
	int result = 0;
	if (*str != 0)
	{
		int sign = 1;
		if (*str == '-')
		{
			sign = -1;
			str++;
		}
		while (*str != 0)
		{
			if (*str < '0' || *str > '9')
			{
				break;
			}
			else
			{
				int num = *str - 48;
				result *= 10;
				result += num;
			}
			str++;
		}
		result *= sign;
	}
	return result;
}

bool MatchString(char *buffer, char *toMatch)
{
	bool doTheyMatch = true;
	while (*buffer && *toMatch)
	{
		if (*buffer != *toMatch)
		{
			doTheyMatch = false;
			break;
		}
		buffer++;
		toMatch++;
	}
	if (!*buffer && *toMatch)
	{
		doTheyMatch = false;
	}

	return doTheyMatch;
}

#define ADVANCE_BY(x, y) x += y
#define ADVANCE_THROUGH_WHITESPACE(x) while(*x == ' ') x++;
#define ADVANCE_TILL(x, y) while(!MatchString(x,y)) x++;
Font Loaders::LoadFont(char *fontFileName)
{
	Font resultFont = {};

	File fontFile = ReadFile(fontFileName);
	char *fileContent = (char *)fontFile.data;
	char *currentChar = fileContent;
	uint32_t fileSize = fontFile.size;
	while (1)
	{
		bool endOfFile = currentChar - fileContent >= fileSize;
		while (!endOfFile &&
			(!MatchString(currentChar, "char") || MatchString(currentChar, "chars")))
		{
			if (MatchString(currentChar, "lineHeight="))
			{
				ADVANCE_BY(currentChar, 11);
				ADVANCE_THROUGH_WHITESPACE(currentChar);
				resultFont.fontHeight = (float)StringToInt(currentChar);
			}
			else if (MatchString(currentChar, "padding="))
			{
				ADVANCE_BY(currentChar, 8);
				ADVANCE_THROUGH_WHITESPACE(currentChar);
				resultFont.paddingTop = (float)StringToInt(currentChar);
				ADVANCE_TILL(currentChar, ",");
				ADVANCE_BY(currentChar, 1);
				resultFont.paddingRight = (float)StringToInt(currentChar);
				ADVANCE_TILL(currentChar, ",");
				ADVANCE_BY(currentChar, 1);
				resultFont.paddingBot = (float)StringToInt(currentChar);
				ADVANCE_TILL(currentChar, ",");
				ADVANCE_BY(currentChar, 1);
				resultFont.paddingLeft = (float)StringToInt(currentChar);
			}
			else
			{
				currentChar++;
			}
			endOfFile = currentChar - fileContent >= fileSize;
		}
		if (endOfFile)
		{
			break;
		}
		char *line = currentChar;
		char ascii;
		Glyph glyph;
		int filledParts = 0;
		while (*line != '\n')
		{
			if (MatchString(line, "id="))
			{
				ADVANCE_BY(line, 3);
				ADVANCE_THROUGH_WHITESPACE(line);
				ascii = StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "x="))
			{
				ADVANCE_BY(line, 2);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.coord.x = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "y="))
			{
				ADVANCE_BY(line, 2);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.coord.y = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "width="))
			{
				ADVANCE_BY(line, 6);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.size.x = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "height="))
			{
				ADVANCE_BY(line, 7);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.size.y = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "xoffset="))
			{
				ADVANCE_BY(line, 8);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.offset.x = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "yoffset="))
			{
				ADVANCE_BY(line, 8);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.offset.y = (float)StringToInt(line);
				filledParts++;
			}
			if (MatchString(line, "xadvance="))
			{
				ADVANCE_BY(line, 9);
				ADVANCE_THROUGH_WHITESPACE(line);
				glyph.advance = (float)StringToInt(line);
				filledParts++;
			}
			line++;
		}

		resultFont.glyphs[ascii] = glyph;

		currentChar++;
	}

	ReleaseFile(fontFile);

	return resultFont;
}
