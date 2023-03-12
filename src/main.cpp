#include "DoomFire.h"
#include "DoomLogo.h"
#include "KeyboardController.h"
#include "LcdController.h"
#include "Pins.h"

#include <pico/bootrom.h>
#include <pico/stdlib.h>

#include <cstring>
#include <stdio.h>

/**

RUN MANUALLY IN THE FOLDER ERROR THIS COMMAND:

python D:/code/others/pico/pico-sdk/src/rp2_common/boot_stage2/pad_checksum -s 0xffffffff bs2_default.bin bs2_default_padded_checksummed.S

 */

static constexpr uint32_t k_fireWidth{LcdController::k_width};
static constexpr uint32_t k_fireHeight{LcdController::k_height / 2};

constexpr uint16_t k_logoStartRow{LcdController::k_height};
constexpr uint16_t k_logoStopRow{30};

enum class InputAction
{
	Restart,
	GotoBootLoader,
	SwitchFire,
	NextEffect,
};

std::optional<InputAction> ProcessInput(KeyboardController& controller)
{
	controller.Update();

	if (controller.HaStateChanged(Key::KeyA) && controller.IsPressed(Key::KeyA))
	{
		return InputAction::SwitchFire;
	}
	if (controller.HaStateChanged(Key::KeyB) && controller.IsPressed(Key::KeyB))
	{
		return InputAction::NextEffect;
	}
	if (controller.HaStateChanged(Key::KeyX) && controller.IsPressed(Key::KeyX))
	{
		return InputAction::Restart;
	}
	if (controller.HaStateChanged(Key::KeyY) && controller.IsPressed(Key::KeyY))
	{
		return InputAction::GotoBootLoader;
	}

	return std::nullopt;
}

void ExecFireAndLogoEffect(LcdController& lcdCtrl, KeyboardController& keyboardCtrl)
{
	DoomFire doomFire{k_fireWidth, k_fireHeight};
	DoomLogo doomLogo{k_logoStartRow, k_logoStopRow};

	const DoomLogo::Descriptor& logoDesc = doomLogo.GetDescriptor();

	// Two rows are computed at the same time and two buffers to allow double buffers.
	constexpr size_t rgbBufferLength{k_fireWidth * 2};

	// Double buffer, while one is sent the other is filled.
	rgb565_t rgbBuffer1[rgbBufferLength];
	rgb565_t rgbBuffer2[rgbBufferLength];

	constexpr size_t rgbBufferSizeInBytes{sizeof(rgbBuffer1)};

	rgb565_t* currentRgbBuffer = rgbBuffer1;

	{
		// Fire effect + doom logo scroll up.
		bool endEffect{false};
		bool running{true};
		bool stopFire{false};
		uint8_t fireIntensity{0xFF};

		constexpr uint32_t k_rowSizeInBytes{k_fireWidth * sizeof(uint16_t)};
		uint16_t* firstRgbRow = currentRgbBuffer;
		uint16_t* secondRgbRow = currentRgbBuffer + k_fireWidth;

		do
		{
			// printf("Doom Fire update\r\n");
			fireIntensity = doomFire.Update();
			doomLogo.Update();

			stopFire = doomLogo.GetCurrentRow() < 80;

			// printf("Send to screen\r\n");
			lcdCtrl.BeginRender({0, 0, LcdController::k_width, LcdController::k_height}, false);

			for (uint16_t row = 0; row < k_fireHeight; row++)
			{
				doomFire.DumpRowRgb565(row, firstRgbRow);
				std::memcpy(secondRgbRow, firstRgbRow, k_rowSizeInBytes);

				{
					const rgb565_t* logoFirstRowPtr = doomLogo.GetRowRgb565(row * 2);
					const rgb565_t* logoSecondRowPtr = doomLogo.GetRowRgb565(row * 2 + 1);
					if (logoFirstRowPtr || logoSecondRowPtr)
					{
						for (uint16_t i = logoDesc.startColumn, j = 0; i < logoDesc.endColumn; ++i, ++j)
						{
							if (!firstRgbRow[i])
							{
								if (logoFirstRowPtr)
								{
									firstRgbRow[i] = logoFirstRowPtr[j];
								}
								if (logoSecondRowPtr)
								{
									secondRgbRow[i] = logoSecondRowPtr[j];
								}
							}
						}
					}
				}

				lcdCtrl.RenderData((const uint8_t*)currentRgbBuffer, rgbBufferSizeInBytes);

				currentRgbBuffer = currentRgbBuffer == rgbBuffer1 ? rgbBuffer2 : rgbBuffer1;
				firstRgbRow = currentRgbBuffer;
				secondRgbRow = currentRgbBuffer + k_fireWidth;
			}

			lcdCtrl.EndRender();

			std::optional<InputAction> action = ProcessInput(keyboardCtrl);
			if (action != std::nullopt)
			{
				switch (action.value())
				{
				case InputAction::GotoBootLoader:
					reset_usb_boot(0, 0);
					break;
				case InputAction::NextEffect:
					endEffect = true;
					break;
				case InputAction::Restart:
					return;
				case InputAction::SwitchFire:
					running = !running;
					running ? doomFire.Start() : doomFire.Stop();
				}
			}

			if (stopFire)
			{
				running = false;
				doomFire.Stop();
			}
		} //
		while (!endEffect && fireIntensity > 1);
	}
}

void ExecLogoFadeOutEffect(LcdController& lcdCtrl, KeyboardController& keyboardCtrl)
{
	DoomLogo doomLogo{k_logoStopRow, k_logoStopRow};
	const DoomLogo::Descriptor& logoDesc = doomLogo.GetDescriptor();

	// The max row's length is the LCD width.
	constexpr size_t rgbBufferLength{LcdController::k_width};

	// Double buffer, while one is sent the other is filled.
	rgb565_t rgbBuffer1[rgbBufferLength];
	rgb565_t rgbBuffer2[rgbBufferLength];

	rgb565_t* currentRgbBuffer = rgbBuffer1;

	bool endEffect{false};

	const uint16_t logoPosY = doomLogo.GetCurrentRow();

	currentRgbBuffer = rgbBuffer1;

	// Fade out the doom logo
	for (uint8_t i = 0; i <= 32  && !endEffect; ++i)
	{
		lcdCtrl.BeginRender({logoDesc.startColumn, logoPosY, logoDesc.width, logoDesc.height}, false);

		for (uint32_t row = logoPosY; row < logoPosY + logoDesc.height; row++)
		{
			const rgb565_t* logoRgbRow = doomLogo.GetRowRgb565(row);
			for (uint32_t j = 0; j < logoDesc.width; ++j)
			{
				currentRgbBuffer[j] = to565(fadeOut(toExtended565(logoRgbRow[j]), 32 - i));
			}

			lcdCtrl.RenderData((const uint8_t*)currentRgbBuffer, logoDesc.width * sizeof(rgb565_t));
			currentRgbBuffer = currentRgbBuffer == rgbBuffer1 ? rgbBuffer2 : rgbBuffer1;
		}

		lcdCtrl.EndRender();

		std::optional<InputAction> action = ProcessInput(keyboardCtrl);
		if (action != std::nullopt)
		{
			switch (action.value())
			{
			case InputAction::GotoBootLoader:
				reset_usb_boot(0, 0);
				break;
			case InputAction::NextEffect:
				endEffect = true;
				break;
			case InputAction::Restart:
				return;
			case InputAction::SwitchFire:
				// Nothing to do for this case.
				break;
			}
		}

		sleep_ms(25);
	}
}

void Execute(LcdController& lcdCtrl, KeyboardController& keyboardCtrl)
{
	ExecFireAndLogoEffect(lcdCtrl, keyboardCtrl);

	sleep_ms(500);

	ExecLogoFadeOutEffect(lcdCtrl, keyboardCtrl);
}


int main()
{
	stdio_init_all();

	LcdController lcdCtrl;
	lcdCtrl.Init(50, ScanDir::Horizontal);

	lcdCtrl.Clear(0);

	KeyboardController keyboardCtrl;
	keyboardCtrl.Init();

	while (true)
	{
		Execute(lcdCtrl, keyboardCtrl);
		lcdCtrl.Clear(0);
		sleep_ms(2000);
	}

	return 0;
}
