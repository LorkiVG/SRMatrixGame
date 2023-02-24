// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// path
#define PATH_DOT_DISTANCE  6
#define PATH_DOT_SPEED     0.007f
#define PATH_DOT_RATE      500
#define PATH_SIZE          1.5f
#define PATH_COLOR         0xFFFFFF10
#define PATH_HIDE_DISTANCE 30


//___________________________________________________________________________________________________
/////////////////////////////////////////////////////////////////////////////////////////////////////

struct SPathDot
{
    SPathDot *next;
    SPathDot *prev;

    CSprite  dot;
    float       pos;
};

class CMatrixEffectPath : public CMatrixEffect
{
    dword     m_Kill = 0;

    CBuf      m_Points;
    CBuf      m_Dirs;
    CBuf      m_Lens;

    int       m_DotsCnt = 0;
    int       m_DotsMax = 0;
    SPathDot* m_Dots = nullptr;

    int       m_Cnt = 0;
    float     m_Len = 0.0f;
    float    _m_Len = 0.0f;

    //float   m_Time = 0.0f;
    float     m_Tact = 0.0f;
    float     m_NextTact = 0.0f;

    SPathDot* m_First = nullptr;
    SPathDot* m_Last = nullptr;

    float     m_Angle = 0.0f;
    float     m_Barier = PATH_HIDE_DISTANCE;


    CMatrixEffectPath(const D3DXVECTOR3* pos, int cnt);
	virtual ~CMatrixEffectPath();

    void UpdateData();

public:
    friend class CMatrixEffect;

    void GetPos(D3DXVECTOR3* out, float t);
    void AddPos(const D3DXVECTOR3& pos);

    virtual void BeforeDraw();
    virtual void Draw();
    virtual void Tact(float step);
    virtual void Release();

    virtual int Priority() { return MAX_EFFECT_PRIORITY; };

    void Kill() { m_Kill = true; }
};

