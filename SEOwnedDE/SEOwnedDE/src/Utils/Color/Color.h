#pragma once

#include <string>
#include <sstream>

struct Color_t
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0;

	std::string toHexStr();
	std::wstring toHexStrW();
};

namespace Colors
{
	constexpr Color_t RED = { 255, 0, 0, 255 };
	constexpr Color_t GREEN = { 0, 255, 0, 255 };
	constexpr Color_t BLUE = { 0, 0, 255, 255 };
	constexpr Color_t WHITE = { 255, 255, 255, 255 };
	constexpr Color_t BLACK = { 0, 0, 0, 255 };
}

namespace ColorUtils
{
	inline float ToFloat(unsigned char x)
	{
		return static_cast<float>(x) / 255.0f;
	}

	inline unsigned long ToDWORD(Color_t x)
	{
		return (x.r & 0xFF) << 24 | (x.g & 0xFF) << 16 | (x.b & 0xFF) << 8 | x.a & 0xFF;
	}

	inline Color_t Rainbow(float offset, float rate = 1.f)
	{
		const int r = std::lround(std::cos(offset * rate + 0.0f) * 127.5f + 127.5f);
		const int g = std::lround(std::cos(offset * rate + 2.0f) * 127.5f + 127.5f);
		const int b = std::lround(std::cos(offset * rate + 4.0f) * 127.5f + 127.5f);

		return Color_t{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(255) };
	}

	Color_t HSLToRGB(float h, float s, float l);
	Color_t Mult(Color_t clr, float mult);
}
