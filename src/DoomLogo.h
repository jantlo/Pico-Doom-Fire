#pragma once

#include "LcdController.h"
#include "Utils.h"

class DoomLogo
{
public:
	struct Descriptor
	{
		uint16_t width;
		uint16_t height;
		uint16_t startColumn;
		uint16_t endColumn;
	};

public:
	DoomLogo(uint16_t startRow, uint16_t stopRow);

	const Descriptor& GetDescriptor() const;

	bool Update();

	uint16_t GetCurrentRow() const;

	const rgb565_t* GetRowRgb565(uint32_t i_row) const;

private:
	Descriptor m_descriptor;
	uint16_t m_row;
	uint16_t m_stopRow;
};
