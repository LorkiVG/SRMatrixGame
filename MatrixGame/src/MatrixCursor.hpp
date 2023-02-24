// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixMap.hpp"

#define CURSOR_VISIBLE          1
#define CURSOR_REVERSEANIM      2

class CMatrixCursor : public CMain
{
    dword        m_CursorFlags = 0;
    CPoint       m_Pos = { 0, 0 };
    CPoint       m_HotSpot = { 0, 0 };
    const wchar* m_CurCursor = nullptr;
    CTextureManaged* m_CursorTexture = nullptr;
    int          m_CursorSize = 0;
    HICON*       m_CursorIcons = nullptr;
    HCURSOR      m_OldCursor;
    int          m_FramesCnt = 0;
    int          m_FrameInc = 0;
    int          m_Frame = 0;
    int          m_NextCursorTime = 0;
    int          m_CursorTimePeriod = 0;
    int          m_CursorInTexLine = 0;
    float        m_TexSizeXInversed = 0.0f, m_TexSizeYInversed = 0.0f;
    float        m_u0 = 0.0f, m_v0 = 0.0f, m_u1 = 0.0f, m_v1 = 0.0f;

    int          m_Time = 0;

public:
    CMatrixCursor()
    {
        //memset(this, 0, sizeof(CMatrixCursor));
        m_OldCursor = SetCursor(nullptr);
    }
    ~CMatrixCursor() { Clear(); }

    void CalcUV();

    void Draw();
    
    void Select(const wchar* name);
    void Tact(int ms);
    void Clear();
    void SetVisible(bool flag);

    const CPoint& GetPos() const { return m_Pos; }
    int           GetPosX() const { return m_Pos.x; }
    int           GetPosY() const { return m_Pos.y; }
    void          SetPos(const CPoint& pos) { SetPos(pos.x, pos.y); }
    void          SetPos(int xx, int yy) { m_Pos.x = xx; m_Pos.y = yy; }
};
