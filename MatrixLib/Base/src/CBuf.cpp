// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CBuf.hpp"
#include "CException.hpp"
#include "Mem.hpp"
#include "CFile.hpp"

namespace Base {

void CBuf::Clear()
{
	m_Len = 0;
	m_Pointer = 0;
}

void CBuf::ClearFull(void)
{
	if(m_Buf != nullptr)
	{
		HFree(m_Buf, m_Heap);
		m_Buf = nullptr;
	}

	m_Len = 0;
	m_Max = 0;
	m_Pointer = 0;
}

/*
inline void CBuf::TestGet(int len)
{
	if(m_Pointer + len > m_Len) ERROR_E; 
}
*/

/*
inline void CBuf::TestAdd(int len)
{
	if(m_Pointer + len > m_Max)
	{
		m_Max = m_Pointer + len + m_Add; 
		m_Buf = (byte*)HAllocEx(m_Buf, m_Max, m_Heap); 
	}
	m_Len += len;
}
*/

/*
inline void CBuf::Pointer(int zn)
{
	if(zn < 0 || zn > m_Len) ERROR_E;
	m_Pointer = zn;
}
*/

void CBuf::Len(int zn)
{
	if(zn <= 0)
	{
		Clear();
		return;
	}

	m_Len = m_Max = zn;
	m_Buf = (byte*)HAllocEx(m_Buf, m_Max, m_Heap);
	if(m_Pointer < 0) m_Pointer = 0;
	else if(m_Pointer > m_Len) m_Pointer = m_Len;
}

int CBuf::StrLen()
{
	int len = 0;
	for(int i = m_Pointer; i < m_Len; ++i, ++len)
	{
		if(*(char*)(m_Buf + i) == 0) return len;
	}
	return 0;
}

int CBuf::WStrLen()
{
	int len = 0;
	for(int i = m_Pointer; i + 1 < m_Len; i += 2, ++len)
	{
		if(*(wchar*)(m_Buf + i) == 0) return len;
	}
	return 0;
}

int CBuf::StrTextLen()
{
	int len = 0;
	for(int i = m_Pointer; i < m_Len; i++, ++len)
	{
		char ch = *(char*)(m_Buf + i);
		if(ch == 0 || ch == 0x0d || ch == 0x0a) return len;
	}
	return len;
}

int CBuf::WStrTextLen()
{
	int len = 0;
	for(int i = m_Pointer; i + 1 < m_Len; i += 2, ++len)
	{
		wchar ch = *(wchar*)(m_Buf + i);
		if(ch == 0 || ch == 0x0d || ch == 0x0a) return len;
	}
	return len;
}

void CBuf::LoadFromFile(const wchar* filename, int len)
{
	ClearFull();
	CFile file(filename, len);
	file.OpenRead();
	Len(file.Size());
	file.Read(m_Buf, m_Len);
	file.Close();
}

void CBuf::SaveInFile(const wchar* filename, int len) const
{
	if(m_Len < 0) return;

	CFile file(filename, len);
	file.Create();
	file.Write(m_Buf, m_Len);
	file.Close();
}

}