#include "DoomLogo.h"

#include "DoomSprite.h"

const static uint16_t k_startColumn{(LcdController::k_width - uint16_t(doom_sprite::k_width)) / 2};
const static uint16_t k_endColumn{uint16_t(LcdController::k_width - k_startColumn)};

DoomLogo::DoomLogo(uint16_t startRow, uint16_t stopRow)
	: m_descriptor{doom_sprite::k_width, doom_sprite::k_height, k_startColumn, k_endColumn}
	, m_row(startRow)
	, m_stopRow(stopRow)
{
}

const DoomLogo::Descriptor& DoomLogo::GetDescriptor() const
{
	return m_descriptor;
}

bool DoomLogo::Update()
{
	m_row = m_row > m_stopRow ? (m_row - 1) : m_stopRow;
	return m_row <= m_stopRow;
}

uint16_t DoomLogo::GetCurrentRow() const
{
	return m_row;
}

const rgb565_t* DoomLogo::GetRowRgb565(uint32_t i_row) const
{
	if (i_row < m_row)
	{
		return nullptr;
	}

	uint32_t row = i_row - m_row;
	if (row >= m_descriptor.height)
	{
		return nullptr;
	}

	return doom_sprite::colors + doom_sprite::k_width * row;
}
