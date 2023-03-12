#pragma once

#include <cstdint>
#include <array>

enum class Key : uint8_t
{
	Left,
	Right,
	Up,
	Down,
	Ctrl,
	KeyA,
	KeyB,
	KeyX,
	KeyY,
};

class KeyboardController
{
public:
	static constexpr uint32_t k_keysCount{9};

public:
	void Init();

	bool IsPressed(Key key) const;
	bool IsReleased(Key key) const;
	bool HaStateChanged(Key key) const;

	void Update();

private:
	struct KeyState
	{
		uint8_t pin;
		bool pressed{false};
		bool stateChanged{false};
	};

private:
	void CheckKey(KeyState& keyState);

private:
	std::array<KeyState, k_keysCount> m_keysState;
};

inline bool KeyboardController::IsPressed(Key key) const
{
	return m_keysState[uint32_t(key)].pressed;
}

inline bool KeyboardController::IsReleased(Key key) const
{
	return !m_keysState[uint32_t(key)].pressed;
}

inline bool KeyboardController::HaStateChanged(Key key) const
{
	return m_keysState[uint32_t(key)].stateChanged;
}
