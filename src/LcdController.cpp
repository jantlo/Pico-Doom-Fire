#include "LcdController.h"

#include "Pins.h"

#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>

#include <algorithm>
#include <cassert>
#include <initializer_list>

enum class Cmd : uint8_t
{
	Nop = 0x00,
	SoftReset = 0x1,
	GetDisplayId = 0x4,
	GetDisplayStatus = 0x9,
	GetDisplayPowerMode = 0xA,
	GetDisplayMADCtrl = 0x0B,
	GetDisplayImgMode = 0x0D,
	GetDisplaySignalMode = 0x0E,
	GetDisplaySelfDiagResult = 0x0F,
	SleepIn = 0x10,
	SleepOut = 0x11,
	InversionOff = 0x20,
	InversionOn = 0x21,
	GammaSet = 0x26,
	DisplayOff = 0x28,
	DisplayOn = 0x29,
	ColumnAddrSet = 0x2A,
	RowAddrSet = 0x2B,
	MemoryWrite = 0x2C,
	MemoryRead = 0x2E,
	PartialArea = 0x30,
	VertScrollDef = 0x33,
	TearingEffectOff = 0x34,
	TearingEffectOn = 0x35,
	MemDataAccessCtrl = 0x36,
	VertScrollStartAddr = 0x37,
	IdleModeOff = 0x38,
	IdleModeOn = 0x39,
	SetColorMode = 0x3A,
	WriteMemContinue = 0x3C,
	ReadMemContinue = 0x3E,
	SetTearScanline = 0x44,
	GetTearScanline = 0x45,
	SetDisplayBrightness = 0x51,
	GetDisplayBrightness = 0x52,
	SetCtrlDisplay = 0x53,
	GetCtrlDisplay = 0x54,
	SetCACE = 0x55,
	GetCACE = 0x56,
	SetCACEMinBrightness = 0x5E,
	GetCACEMinBrightness = 0x5F,
	GetABCSDR = 0x68,
	GetId1 = 0xDA,
	GetId2 = 0xDB,
	GetId3 = 0xDC,
	SetRamCtrl = 0xB0,
	SetRgbCtrl = 0xB1,
	SetPorchSetting = 0xB2,
	SetFrameRateCtr1 = 0xB3,
	SetPartialCtrl = 0xB5,
	SetGateCtrl = 0xB7,
	SetGateOnTimingAdj = 0xB8,
	EnableDigitalGamma = 0xBA,
	SetVCOMS = 0xBB,
	SetLCMCtrl = 0xC0,
	EnableVDVAndVRH = 0xC2,
	SetVRHS = 0xC3,
	SetVDV = 0xC3,
	SetFrameRateCtrlInNormalMode = 0xC6,
	SetPowerCtrl1 = 0xD0,
	SetPositiveVoltGammaCtrl = 0xE0,
	SetNegativeVoltGammaCtrl = 0xE1,
};

#define SPI_PORT spi1

namespace
{
struct Command
{
	Cmd cmd;
	std::initializer_list<uint8_t> data;
};

enum class PinMode : uint8_t
{
	In = GPIO_IN,
	Out = GPIO_OUT,
};

static inline void gpio_init_pin(uint16_t pin, PinMode mode)
{
	gpio_init(pin);
	gpio_set_dir(pin, bool(mode));
}

static inline void spi_write_byte_blocking(uint8_t value)
{
	spi_write_blocking(SPI_PORT, &value, 1);
}

static inline void Reset()
{
	gpio_put(pins::k_lcdReset, 1);
	sleep_ms(100);
	gpio_put(pins::k_lcdReset, 0);
	sleep_ms(100);
	gpio_put(pins::k_lcdReset, 1);
	sleep_ms(100);
}

static inline void SendCommand(Cmd reg)
{
	gpio_put(pins::k_lcdDataCmd, 0);
	gpio_put(pins::k_lcdChipSelect, 0);
	spi_write_byte_blocking(uint8_t(reg));
	gpio_put(pins::k_lcdChipSelect, 1);
}

static inline void SendCommands_8bit(const std::initializer_list<Command>& commands)
{
	gpio_put(pins::k_lcdChipSelect, 0);

	gpio_put(pins::k_lcdDataCmd, 0);
	for (const Command& cmd : commands)
	{
		spi_write_byte_blocking(uint8_t(cmd.cmd));

		if (cmd.data.size() > 0)
		{
			gpio_put(pins::k_lcdDataCmd, 1);
			for (uint8_t v : cmd.data)
			{
				spi_write_byte_blocking(v);
			}
			gpio_put(pins::k_lcdDataCmd, 0);
		}
	}

	gpio_put(pins::k_lcdChipSelect, 1);
}

static void SendData_8Bit(uint8_t data)
{
	gpio_put(pins::k_lcdDataCmd, 1);
	gpio_put(pins::k_lcdChipSelect, 0);
	spi_write_byte_blocking(data);
	gpio_put(pins::k_lcdChipSelect, 1);
}

static void SendData_8Bit(const std::initializer_list<uint8_t>& datas)
{
	gpio_put(pins::k_lcdDataCmd, 1);
	gpio_put(pins::k_lcdChipSelect, 0);
	for (uint8_t v : datas)
	{
		spi_write_byte_blocking(v);
	}
	gpio_put(pins::k_lcdChipSelect, 1);
}

static void SendData_16Bit(uint16_t data)
{
	gpio_put(pins::k_lcdDataCmd, 1);
	gpio_put(pins::k_lcdChipSelect, 0);
	spi_write_byte_blocking((data >> 8) & 0xFF);
	spi_write_byte_blocking(data & 0xFF);
	gpio_put(pins::k_lcdChipSelect, 1);
}
} // namespace

LcdController::~LcdController()
{
	WaitAndReleaseDmaResources();
}

void LcdController::Init(uint8_t backlight, ScanDir scanDir)
{
	spi_init(SPI_PORT, 40000 * 1000);

	gpio_set_function(pins::k_lcdClock, GPIO_FUNC_SPI);
	gpio_set_function(pins::k_lcdMOSI, GPIO_FUNC_SPI);

	gpio_init_pin(pins::k_lcdReset, PinMode::Out);
	gpio_init_pin(pins::k_lcdDataCmd, PinMode::Out);
	gpio_init_pin(pins::k_lcdChipSelect, PinMode::Out);
	gpio_init_pin(pins::k_lcdBackLight, PinMode::Out);

	gpio_init_pin(pins::k_lcdChipSelect, PinMode::Out);
	gpio_init_pin(pins::k_lcdBackLight, PinMode::Out);

	gpio_put(pins::k_lcdChipSelect, 1);
	gpio_put(pins::k_lcdDataCmd, 0);
	gpio_put(pins::k_lcdBackLight, 1);

	m_backlight = std::min<uint16_t>(backlight, 100);
	const uint32_t slice_num = pwm_gpio_to_slice_num(pins::k_lcdBackLight);
	pwm_set_chan_level(slice_num, PWM_CHAN_B, m_backlight);

	// Hardware reset
	Reset();

	// Set the resolution and scanning method of the screen
	m_scanDir = scanDir;
	const uint8_t memoryAccessReg = scanDir == ScanDir::Horizontal ? 0x70 : 0x0;

	// clang-format off
	SendCommands_8bit({
		{Cmd::MemDataAccessCtrl, {memoryAccessReg}}, // Set the read / write scan direction of the frame memory MX, MY, RGB mode
		{Cmd::SetColorMode, {0x05}}, // Set the initialization register
		{Cmd::SetPorchSetting, {0x0C, 0x0C, 0x33}},
		{Cmd::SetGateCtrl, {0x35}}, // Gate Control
		{Cmd::SetVCOMS, {0x19}}, // VCOM Setting
		{Cmd::SetLCMCtrl, {0x2C}}, // LCM Control
		{Cmd::EnableVDVAndVRH, {0x01}}, // VDV and VRH Command Enable
		{Cmd::SetVRHS, {0x12}}, // VRH Set
		{Cmd::SetVDV, {0x20}}, // VDV Set
		{Cmd::SetFrameRateCtrlInNormalMode, {0x0F}}, // Frame Rate Control in Normal Mode
		{Cmd::SetPowerCtrl1, {0xA4, 0xA1}}, // Power Control 1
		{Cmd::SetPositiveVoltGammaCtrl, {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}}, // Positive Voltage Gamma Control
		{Cmd::SetNegativeVoltGammaCtrl, {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}}, // Negative Voltage Gamma Control
		{Cmd::SetRamCtrl,{0x00, 0b11111000}}, // Force little endian to avoid swap the word data.
		{Cmd::InversionOn, {}}, // Display Inversion On
		{Cmd::SleepOut, {}}, // Sleep Out
		{Cmd::DisplayOn, {}} // Display On
	});
	// clang-format on
}

void LcdController::BeginRender(const Viewport& viewport, bool blocking)
{
	assert(!m_rendering);
	assert(m_dmaInfo == std::nullopt);

	m_rendering = true;

	const uint16_t endX = viewport.x + viewport.width - 1;
	const uint16_t endY = viewport.y + viewport.height - 1;

	SendCommands_8bit({
		{Cmd::ColumnAddrSet, {uint8_t(viewport.x >> 8), uint8_t(viewport.x & 0xFF), uint8_t(endX >> 8), uint8_t(endX & 0xFF)}}, //
		{Cmd::RowAddrSet, {uint8_t(viewport.y >> 8), uint8_t(viewport.y & 0xFF), uint8_t(endY >> 8), uint8_t(endY & 0xFF)}},
		{Cmd::MemoryWrite, {}} //
	});

	if (!blocking)
	{
		if (m_dmaInfo == std::nullopt)
		{
			const uint32_t dmaChannel = dma_claim_unused_channel(true);
			dma_channel_config dmaCfg = dma_channel_get_default_config(dmaChannel);
			channel_config_set_transfer_data_size(&dmaCfg, DMA_SIZE_8);
			channel_config_set_dreq(&dmaCfg, spi_get_dreq(SPI_PORT, true));
			channel_config_set_read_increment(&dmaCfg, true);
			channel_config_set_write_increment(&dmaCfg, false);
			m_dmaInfo = DmaInfo{dmaChannel, dmaCfg};
		}
	}
	else
	{
		WaitAndReleaseDmaResources();
	}

	gpio_put(pins::k_lcdDataCmd, 1);
	gpio_put(pins::k_lcdChipSelect, 0);
}

void LcdController::RenderData(const uint8_t* dataPtr, uint32_t length)
{
	if (m_dmaInfo != std::nullopt)
	{
		dma_channel_wait_for_finish_blocking(m_dmaInfo->channel);

		dma_channel_configure(m_dmaInfo->channel,		 // Channel to be configured
							  &m_dmaInfo->config,		 // The configuration we just created
							  &spi_get_hw(SPI_PORT)->dr, // write address (SPI data register)
							  dataPtr,					 // The initial read address
							  length,					 // Number of transfers
							  true						 // start immediately.
		);
	}
	else
	{
		spi_write_blocking(SPI_PORT, dataPtr, length);
	}
}

void LcdController::EndRender()
{
	assert(m_rendering);

#if 0
	WaitAndReleaseDmaResources();
#endif
	m_rendering = false;

	gpio_put(pins::k_lcdChipSelect, 1);
}

void LcdController::Clear(uint16_t color)
{
	uint16_t pixelsRow[k_width];

	color = ((color << 8) & 0xff00) | (color >> 8);

	for (uint16_t j = 0; j < k_width; j++)
	{
		pixelsRow[j] = color;
	}

	BeginRender({0, 0, k_width, k_height}, true);
	for (uint16_t j = 0; j < k_height; j++)
	{
		RenderData((uint8_t*)pixelsRow, k_width * 2);
	}
	EndRender();
}

void LcdController::Display(const uint16_t* image, bool blocking)
{
	BeginRender({0, 0, k_width, k_height}, blocking);
	RenderData((const uint8_t*)image, k_width * k_height * sizeof(uint16_t));
	EndRender();
}

void LcdController::DrawImage(const Viewport& viewport, const uint16_t* image, bool blocking)
{
	BeginRender(viewport, blocking);
	RenderData((const uint8_t*)image, viewport.width * viewport.height * sizeof(uint16_t));
	EndRender();
}

void LcdController::DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	BeginRender({x, y, x, y}, true);
	RenderData((const uint8_t*)&color, sizeof(color));
	EndRender();
}

inline void LcdController::WaitAndReleaseDmaResources()
{
	if (m_dmaInfo != std::nullopt)
	{
		dma_channel_wait_for_finish_blocking(m_dmaInfo->channel);
		dma_channel_unclaim(m_dmaInfo->channel);
		m_dmaInfo = std::nullopt;
	}
}

