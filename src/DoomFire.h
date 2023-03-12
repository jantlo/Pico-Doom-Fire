#pragma once

#include "LcdController.h"
#include "Utils.h"

class DoomFire
{
public:
	DoomFire(uint16_t width, uint16_t height);
	~DoomFire();

	void Stop();
	void Start();

	uint8_t Update();

	const uint8_t* GetRowIntensities(uint32_t row) const;
	void DumpRowRgb565(uint32_t row, rgb565_t* lineRgb16) const;

private:
	uint16_t m_width;
	uint16_t m_height;
	uint32_t m_bufferSize;

	bool m_fadeOut{false};
	uint8_t m_fadeOutValue{36};
	bool m_hasFire{false};
	uint8_t* m_buffer{nullptr};
};
