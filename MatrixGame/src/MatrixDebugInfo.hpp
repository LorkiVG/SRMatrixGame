// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIXDEBUGINFO_INCLUDE
#define MATRIXDEBUGINFO_INCLUDE


#define MAX_DEBUG_INFO_ITEMS 128

#define DI_DRAWFPS      SETBIT(0)
#define DI_VISOBJ       SETBIT(1)
#define DI_TMEM         SETBIT(2)
#define DI_TARGETCOORD  SETBIT(3)
#define DI_SIDEINFO     SETBIT(4)
#define DI_ACTIVESOUNDS SETBIT(5)
#define DI_FRUSTUMCENTER SETBIT(6)

#define DI_KEY_X    10
#define DI_KEY_Y    10
#define DI_KEY_W    140
#define DI_KEY_H    14
#define DI_VAL_W    200


struct SDIItem
{
    CWStr* key = nullptr;
    CWStr* val = nullptr;
    int    ttl = 0;
    int    bttl = 0;
};

class CMatrixDebugInfo : public CMain
{
    SDIItem    m_Items[MAX_DEBUG_INFO_ITEMS];
    int        m_ItemsCnt = 0;
    ID3DXFont* m_Font = nullptr;

    CPoint     m_Pos = { DI_KEY_X, DI_KEY_Y };

public:
    CMatrixDebugInfo() = default;
    ~CMatrixDebugInfo() { Clear(); };

    void Clear();
    void Draw();
    void Tact(int ms);

    void SetStartPos(const CPoint& pos) { m_Pos = pos; }

    void T(const wchar* key, const wchar* val, int ttl = 3000, int bttl = 0, bool add = false);

    void InitFont();
    void ClearFont() { if(m_Font) { m_Font->Release(); m_Font = nullptr; } };

    void OnLostDevice() { if(m_Font) m_Font->OnLostDevice(); }
    void OnResetDevice() { if(m_Font) m_Font->OnResetDevice(); }
};


#endif