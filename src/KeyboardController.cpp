#include "KeyboardController.h"

#include "Pins.h"

#include <hardware/gpio.h>

static inline void ConfigurePin(uint8_t pin)
{
	gpio_set_dir(pin, GPIO_IN);
	gpio_pull_up(pin); // Need to pull up
}

void KeyboardController::Init()
{
	ConfigurePin(pins::k_keyA);
	ConfigurePin(pins::k_keyB);
	ConfigurePin(pins::k_keyX);
	ConfigurePin(pins::k_keyY);

	ConfigurePin(pins::k_keyUp);
	ConfigurePin(pins::k_keyDowm);
	ConfigurePin(pins::k_keyLeft);
	ConfigurePin(pins::k_keyRight);
	ConfigurePin(pins::k_keyCtrl);

	m_keysState[size_t(Key::Left)] = {pins::k_keyLeft, false, false};
	m_keysState[size_t(Key::Right)] = {pins::k_keyRight, false, false};
	m_keysState[size_t(Key::Up)] = {pins::k_keyUp, false, false};
	m_keysState[size_t(Key::Down)] = {pins::k_keyDowm, false, false};
	m_keysState[size_t(Key::Ctrl)] = {pins::k_keyCtrl, false, false};
	m_keysState[size_t(Key::KeyA)] = {pins::k_keyA, false, false};
	m_keysState[size_t(Key::KeyB)] = {pins::k_keyB, false, false};
	m_keysState[size_t(Key::KeyX)] = {pins::k_keyX, false, false};
	m_keysState[size_t(Key::KeyY)] = {pins::k_keyY, false, false};
}

void KeyboardController::Update()
{
	for (KeyState& state : m_keysState)
	{
		if (gpio_get(state.pin) == 0)
		{
			state.stateChanged = !state.pressed;
			state.pressed = true;
		}
		else
		{
			state.stateChanged = state.pressed;
			state.pressed = false;
		}
	}
}
