// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"

namespace Base {

class CWStr;

#define ERROR_E throw new Base::CException(__FILE__, __LINE__)
#define ERROR_S(s1) { SFT_fun("\rPlanetary battle Exception! " + CStr(s1)); throw new Base::CExceptionStr(__FILE__, __LINE__, s1); }
#define ERROR_S2(s1, s2) { SFT_fun("\rPlanetary battle Exception! " + CStr(s1) + CStr(s2)); throw new Base::CExceptionStr(__FILE__, __LINE__, s1, s2); }
#define ERROR_S3(s1, s2, s3) { SFT_fun("\rPlanetary battle Exception! " + CStr(s1) + CStr(s2) + CStr(s3)); throw new Base::CExceptionStr(__FILE__, __LINE__, s1, s2, s3); }
#define ERROR_S4(s1, s2, s3, s4) { SFT_fun("\rPlanetary battle Exception! " + CStr(s1) + CStr(s2) + CStr(s3) + CStr(s4)); throw new Base::CExceptionStr(__FILE__, __LINE__, s1, s2, s3, s4); }

#ifdef ASSERT_OFF
	#define ASSERT(zn)
#else
#ifdef _TRACE
#define ASSERT(zn) { if(!(zn)) ERROR_E; }
#else
#define ASSERT(zn) { if(!(zn)) __debugbreak(); }
#endif
#endif

//lint -e1712
class BASE_API CException
{
	const char* m_File;
	int m_Line;
    char call_trace[65536];

public:
	CException(const char* file, int line) : m_File(file), m_Line(line) { CreateCallTrace(); }
	virtual ~CException() = default;

    void CreateCallTrace();

	virtual CWStr Info();
};

class BASE_API CExceptionStr : public CException
{
	wchar m_Str[128];

public:
	CExceptionStr(const char* file, int line, const wchar* str, const wchar* str2 = nullptr, const wchar* str3 = nullptr, const wchar* str4 = nullptr);

	virtual CWStr Info();
};
//lint +e1712
}
