#include "DoomFire.h"

#include "Pins.h"
#include "Utils.h"

#include <cstdlib>
#include <random>

#include <algorithm>

static constexpr rgb565_t s_rgbs[]{
	0x0,
	to565(0x07, 0x07, 0x07), // 0
	to565(0x1F, 0x07, 0x07), // 1
	to565(0x2F, 0x0F, 0x07), // 2
	to565(0x47, 0x0F, 0x07), // 3
	to565(0x57, 0x17, 0x07), // 4
	to565(0x67, 0x1F, 0x07), // 5
	to565(0x77, 0x1F, 0x07), // 6
	to565(0x8F, 0x27, 0x07), // 7
	to565(0x9F, 0x2F, 0x07), // 8
	to565(0xAF, 0x3F, 0x07), // 9
	to565(0xBF, 0x47, 0x07), // 10
	to565(0xC7, 0x47, 0x07), // 11
	to565(0xDF, 0x4F, 0x07), // 12
	to565(0xDF, 0x57, 0x07), // 13
	to565(0xDF, 0x57, 0x07), // 14
	to565(0xD7, 0x5F, 0x07), // 15
	to565(0xD7, 0x5F, 0x07), // 16
	to565(0xD7, 0x67, 0x0F), // 17
	to565(0xCF, 0x6F, 0x0F), // 18
	to565(0xCF, 0x77, 0x0F), // 19
	to565(0xCF, 0x7F, 0x0F), // 20
	to565(0xCF, 0x87, 0x17), // 21
	to565(0xC7, 0x87, 0x17), // 22
	to565(0xC7, 0x8F, 0x17), // 23
	to565(0xC7, 0x97, 0x1F), // 24
	to565(0xBF, 0x9F, 0x1F), // 25
	to565(0xBF, 0x9F, 0x1F), // 26
	to565(0xBF, 0xA7, 0x27), // 27
	to565(0xBF, 0xA7, 0x27), // 28
	to565(0xBF, 0xAF, 0x2F), // 29
	to565(0xB7, 0xAF, 0x2F), // 30
	to565(0xB7, 0xB7, 0x2F), // 31
	to565(0xB7, 0xB7, 0x37), // 32
	to565(0xCF, 0xCF, 0x6F), // 33
	to565(0xDF, 0xDF, 0x9F), // 34
	to565(0xEF, 0xEF, 0xC7), // 35
	to565(0xFF, 0xFF, 0xFF), // 36
};

constexpr uint8_t k_minValue{0};
constexpr uint8_t k_maxValue{(sizeof(s_rgbs) / sizeof(uint16_t)) - 1};

DoomFire::DoomFire(uint16_t width, uint16_t height)
	: m_width{width}
	, m_height{height}
	, m_bufferSize{uint32_t(m_width) * m_height}
{
	m_buffer = (uint8_t*)malloc(m_bufferSize);

	std::fill_n(m_buffer, m_bufferSize - m_width, 0);

	Start();
}

DoomFire::~DoomFire()
{
	free(m_buffer);
	m_buffer = nullptr;
}

void DoomFire::Stop()
{
	if (!m_fadeOut)
	{
		m_fadeOut = true;
		m_fadeOutValue = k_maxValue;
	}
}

void DoomFire::Start()
{
	m_fadeOut = false;
	m_fadeOutValue = k_maxValue;

	uint8_t* bottomLine = m_buffer + m_bufferSize - m_width;
	std::fill_n(bottomLine, m_width, k_maxValue);
}

uint8_t DoomFire::Update()
{
	uint8_t intensity{k_minValue};

	uint8_t* src = m_buffer + m_width; // Points to the second row.
	const uint8_t* end = m_buffer + m_bufferSize;
	for (; src < end; ++src)
	{
		uint8_t pixel = *src;
		if (pixel == k_minValue)
		{
			uint8_t* dst = src - m_width;
			*dst = k_minValue;
		}
		else
		{
			const uint8_t randIdx = std::abs(std::rand()) & 3;
			uint8_t* dst = src - randIdx + 1 - m_width;
			const uint8_t v = pixel - (randIdx & 1);
			*dst = v;
			intensity = std::max(intensity, v);
		}
	}

	if (m_fadeOut && m_fadeOutValue > k_minValue)
	{
		uint8_t* bottomLine = m_buffer + m_bufferSize - m_width;
		std::fill_n(bottomLine, m_width, m_fadeOutValue);
		--m_fadeOutValue;
		intensity = std::max(intensity, m_fadeOutValue);
	}

	return intensity;
}

const uint8_t* DoomFire::GetRowIntensities(uint32_t row) const
{
	return m_buffer + row * m_width;
}

void DoomFire::DumpRowRgb565(uint32_t row, rgb565_t* lineRgb16) const
{
	const uint8_t* linePtr = m_buffer + row * m_width;
	for (uint16_t x = 0; x < m_width; ++x, ++linePtr)
	{
		lineRgb16[x] = s_rgbs[*linePtr];
	}
}
