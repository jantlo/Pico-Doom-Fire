#pragma once

#include <hardware/dma.h>

#include <cstdint>
#include <optional>

enum class ScanDir : uint8_t
{
	Horizontal = 0,
	Vertical = 1
};

struct Viewport
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

class LcdController
{
public:
	static constexpr uint16_t k_height{240};
	static constexpr uint16_t k_width{240};

public:
	~LcdController();

	void Init(uint8_t backlight, ScanDir scanDir);
	void Clear(uint16_t Color);
	void Display(const uint16_t* image, bool blocking);
	void DrawImage(const Viewport& viewport, const uint16_t* image, bool blocking);
	void DrawPoint(uint16_t x, uint16_t y, uint16_t color);

	void BeginRender(const Viewport& viewport, bool blocking);

	void RenderData(const uint8_t* dataPtr, uint32_t length);
	void EndRender();

private:
	void WaitAndReleaseDmaResources();

private:
	struct DmaInfo
	{
		uint32_t channel;
		dma_channel_config config;
	};

private:
	ScanDir m_scanDir{ScanDir::Horizontal};
	uint8_t m_backlight{50};

	bool m_rendering{false};
	std::optional<DmaInfo> m_dmaInfo;
};
