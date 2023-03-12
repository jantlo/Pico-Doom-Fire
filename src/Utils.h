#pragma once

#include <algorithm>
#include <cstdint>

using rgb565_t = uint16_t;
using extended_rgb565_t = uint32_t;

constexpr rgb565_t to565(uint8_t r, uint8_t g, uint8_t b)
{
	constexpr rgb565_t maskRed{0b1111100000000000};
	constexpr rgb565_t maskGreen{0b0000011111100000};
	constexpr rgb565_t maskBlue{0b0000000000011111};
	return ((rgb565_t(r) << 8) & maskRed) | ((rgb565_t(g) << 3) & maskGreen) | ((rgb565_t(b) >> 3) & maskBlue);
}

constexpr extended_rgb565_t toExtended565(uint8_t r, uint8_t g, uint8_t b)
{
	constexpr extended_rgb565_t maskRed{0b00000111110000000000000000000000};
	constexpr extended_rgb565_t maskGreen{0b00000000000000001111110000000000};
	constexpr extended_rgb565_t maskBlue{0b00000000000000000000000000011111};

	return ((extended_rgb565_t(r) << 19) & maskRed) | ((extended_rgb565_t(g) << 8) & maskGreen) | (extended_rgb565_t(b >> 3) & maskBlue);
}

constexpr rgb565_t to565(extended_rgb565_t value)
{
	constexpr rgb565_t maskRed{0b1111100000000000};
	constexpr rgb565_t maskGreen{0b0000011111100000};
	constexpr rgb565_t maskBlue{0b0000000000011111};

	return (rgb565_t(value >> 11) & maskRed) | (rgb565_t(value >> 5) & maskGreen) | (rgb565_t(value) & maskBlue);
}

constexpr extended_rgb565_t toExtended565(rgb565_t value)
{
	constexpr rgb565_t maskRed{0b1111100000000000};
	constexpr rgb565_t maskGreen{0b0000011111100000};
	constexpr rgb565_t maskBlue{0b0000000000011111};

	return (extended_rgb565_t(value & maskRed) << 11) | (extended_rgb565_t(value & maskGreen) << 5) | (extended_rgb565_t(value & maskBlue));
}

constexpr extended_rgb565_t fadeOut(extended_rgb565_t v1, uint8_t a)
{
	const uint8_t a1 = std::min<uint8_t>(32, a);
	return ((v1 * a1) >> 5) & 0b111110000001111110000011111;
}

constexpr extended_rgb565_t lerp(extended_rgb565_t v1, extended_rgb565_t v2, uint8_t a)
{
	const uint8_t a1 = std::min<uint8_t>(32, a);
	const uint8_t a2 = 32 - a;
	return ((v1 * a1 + v2 * a2) >> 5) & 0b111110000001111110000011111;
}
