// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


// fireplasma

#define FIRE_PLASMA_W 3
#define FIRE_PLASMA_L 7



class CMatrixEffectFirePlasma : public CMatrixEffect
{
    dword             m_UserData = 0;
    FIRE_END_HANDLER  m_Handler = nullptr;
    dword             m_HitMask = 0;
    CMatrixMapStatic* m_Skip = nullptr;

    float m_Speed = 0.0f;
    float m_Prevdist = 0.0f;
    D3DXVECTOR3       m_Pos;
    D3DXVECTOR3       m_Dir;
    D3DXVECTOR3       m_End;

    SEffectHandler    m_Light;

    CSpriteLine   m_BBLine;
    CSprite          m_BB1;
    CSprite          m_BB2;
    CSprite          m_BB3;
    CSprite          m_BB4;

    CMatrixEffectFirePlasma(const D3DXVECTOR3& start, const D3DXVECTOR3& end, float speed, dword hitmask, CMatrixMapStatic* skip, FIRE_END_HANDLER handler, dword user);
	virtual ~CMatrixEffectFirePlasma();

public:
    friend class CMatrixEffect;
    friend class CPolygon;

    virtual void BeforeDraw(void) {}
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);

    virtual int  Priority(void) { return MAX_EFFECT_PRIORITY; };
};


