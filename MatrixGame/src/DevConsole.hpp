// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CWStr.hpp"
#include "MatrixMap.hpp"
#include "MatrixSoundManager.hpp"

#ifndef DEV_CONSOLE_INCLUDE
#define DEV_CONSOLE_INCLUDE

//class CWStr;

typedef void (*CMD_HANDLER)(const Base::CWStr& cmd, const Base::CWStr& params);

struct SCmdItem
{
    const wchar* cmd = nullptr;
    CMD_HANDLER  handler = nullptr;
};


#define DCON_ACTIVE   SETBIT(0)
#define DCON_CURSOR   SETBIT(1)
#define DCON_SHIFT    SETBIT(2)


#define DEV_CONSOLE_CURSOR_FLASH_PERIOD 300

class CDevConsole : public CMain
{
    dword           m_Flags = 0;

    static SCmdItem m_Commands[];

    CWStr           m_Text = (CWStr)L"";
    int             m_CurPos = 0;

    int             m_NextTime = 0;
    int             m_Time = 0;

public:
    CDevConsole() = default;
    ~CDevConsole() = default;

    bool IsActive(void) const;
    void SetActive(bool set);

    void ShowHelp(void);

    void Tact(int ms);
    void Keyboard(int scan, bool down);
};
#endif