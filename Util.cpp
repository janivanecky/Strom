#include "Util.h"

Vector4 Util::HSVtoRGB(Vector4 color)
{
	float h = color.x, s = color.y, v = color.z, a = color.w;
	float min;
	float chroma;
	float hDash;
	float x;

	float r = 0, g = 0, b = 0;
	chroma = s * v;
	hDash = h * 6.0f;
	x = chroma * (1.0f - Math::Abs(Math::Fmod(hDash, 2.0f) - 1.0f));

	if (hDash < 1.0)
	{
		r = chroma;
		g = x;
	}
	else if (hDash < 2.0)
	{
		r = x;
		g = chroma;
	}
	else if (hDash < 3.0)
	{
		g = chroma;
		b = x;
	}
	else if (hDash < 4.0)
	{
		g = x;
		b = chroma;
	}
	else if (hDash < 5.0)
	{
		r = x;
		b = chroma;
	}
	else if (hDash <= 6.0)
	{
		r = chroma;
		b = x;
	}

	min = v - chroma;

	r += min;
	g += min;
	b += min;

	return Vector4(r, g, b, a);
}

Vector4 Util::RGBtoHSV(Vector4 color)
{
	Vector4 result;
	float cMax = MAX(color.x, MAX(color.y, color.z));
	float cMin = MIN(color.x, MIN(color.y, color.z));
	float delta = cMax - cMin;
	float h = 0;
	float coef = 1.0f / 6.0f;
	if (cMax == color.x)
	{
		h = coef * Math::Fmod((color.y - color.z) / delta, 6.0f);
	}
	if (cMax == color.y)
	{
		h = coef * ((color.z - color.x) / delta + 2.0f);
	}
	if (cMax == color.z)
	{
		h = coef * ((color.x - color.y) / delta + 4.0f);
	}

	float s = 0;
	if (cMax == 0)
	{
		s = 0;
	}
	else
	{
		s = delta / cMax;
	}
	if (h < 0)
	{
		h += 1.0f;
	}

	float v = cMax;
	return Vector4(h, s, v, color.w);
}
