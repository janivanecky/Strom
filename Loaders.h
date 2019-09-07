#pragma once
#include "Math.h"

struct DDSTexture
{
	uint32_t width;
	uint32_t height;
	uint32_t mipCount;
	uint32_t bytesPerPixel;
	void *data;
};

struct Glyph
{
	Vector2 coord;
	Vector2 size;
	Vector2 offset;
	float advance;
	bool active;
};

struct Font
{
	Glyph glyphs[128];
	float paddingTop;
	float paddingRight;
	float paddingBot;
	float paddingLeft;
	float fontHeight;
};


namespace Loaders
{
	DDSTexture LoadDDSTexture(char *path);
	Font LoadFont(char *path);
}
