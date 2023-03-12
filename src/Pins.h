#pragma once

#include <cstdint>

/*
LCD 1.3
LCD    =>    Pico
VCC    ->    VSYS
GND    ->    GND
DIN    ->    11
CLK    ->    10
CS     ->    9
DC     ->    8
RST    ->    12
BL     ->    13
A       ->   15
B       ->    17
X       ->    19
Y       ->    21
UP     ->    2
DOWM ->    18
LEFT   ->    16
RIGHT ->    20
CTRL  ->    3
*/

namespace pins
{
constexpr uint8_t k_lcdReset = 12;
constexpr uint8_t k_lcdDataCmd = 8;
constexpr uint8_t k_lcdBackLight = 13;

constexpr uint8_t k_lcdChipSelect = 9;
constexpr uint8_t k_lcdClock = 10;
constexpr uint8_t k_lcdMOSI = 11;

constexpr uint8_t k_lcdScl = 7;
constexpr uint8_t k_lcdSda = 6;

constexpr uint8_t k_keyA = 15;
constexpr uint8_t k_keyB = 17;
constexpr uint8_t k_keyX = 19;
constexpr uint8_t k_keyY = 21;

constexpr uint8_t k_keyUp = 2;
constexpr uint8_t k_keyDowm = 18;
constexpr uint8_t k_keyLeft = 16;
constexpr uint8_t k_keyRight = 20;
constexpr uint8_t k_keyCtrl = 3;
} // namespace pins
