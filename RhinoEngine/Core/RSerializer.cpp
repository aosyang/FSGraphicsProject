//=============================================================================
// RSerializer.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RSerializer.h"

RSerializer::~RSerializer()
{
	Close();
}

void RSerializer::Open(const std::string& filename, ESerializeMode mode)
{
	m_Mode = mode;

	m_FileStream.open(filename, (mode == ESerializeMode::Read ? std::ios::in : std::ios::out) | std::ios::binary);

	if (!m_FileStream.is_open())
		return;
}

bool RSerializer::EnsureHeader(const char* header, UINT size)
{
	if (m_Mode == ESerializeMode::Write)
	{
		m_FileStream.write(header, size);
	}
	else
	{
		assert(!m_FileStream.eof());
		char* pBuf = new char[size];
		m_FileStream.read(pBuf, size);

		for (UINT i = 0; i < size; i++)
		{
			if (pBuf[i] != header[i])
			{
				delete[] pBuf;
				return false;
			}
		}

		delete[] pBuf;
	}

	return true;
}
