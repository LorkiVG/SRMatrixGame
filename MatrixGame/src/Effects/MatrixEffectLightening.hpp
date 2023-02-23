// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


// lightening

#define LIGHTENING_SEGMENT_LENGTH 30
#define LIGHTENING_WIDTH          10

#define SHORTED_SEGMENT_LENGTH    5
#define SHORTED_WIDTH             5

class CMatrixEffectLightening : public CMatrixEffect
{
    D3DXVECTOR3 m_Pos0;
    D3DXVECTOR3 m_Pos1;
    D3DXVECTOR3 m_Dir;

    CMatrixEffectLightening(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, float ttl, float dispers, float width, dword color, int spot_sprite_num = SPR_DISCHARGER_SPOT, int beam_sprite_num = SPR_DISCHARGER_BEAM, bool bp = true);
	virtual ~CMatrixEffectLightening();

    CSpriteLine* m_BL;
    int          m_BL_cnt;

    CSprite*     m_End0;

    float m_SegLen = 0.0f;

    virtual void UpdateData(void);

    float m_TTL;

    float m_Dispersion;
    float m_Width;
    dword m_Color;

    ESpriteTextureSort m_SpotSpriteNum;
    ESpriteTextureSort m_BeamSpriteNum;

    D3DXVECTOR3 m_Perp;

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectShorted;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);

    virtual int  Priority(void) { return 10; };

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1);
};

class CMatrixEffectShorted : public CMatrixEffectLightening
{
    CMatrixEffectShorted(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, float ttl, dword color, int sprite_num);

    float  m_Len = 0.0f;
    float _m_BL_cnt = 0.0f;
    float _m_TTL = 0.0f;

    ESpriteTextureSort m_BeamSpriteNum;

    virtual void UpdateData(void);

public:
    friend class CMatrixEffect;

    virtual int Priority(void) { return 1; };

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1);
};