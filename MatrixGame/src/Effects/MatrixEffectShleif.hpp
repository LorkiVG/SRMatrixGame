// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_EFFECT_SHLEIF
#define MATRIX_EFFECT_SHLEIF

#define SHLEIF_MAX_SMOKES   64
#define SHLEIF_TTL          2000

class CMatrixEffectShleif : public CMatrixEffect
{
    SEffectHandler* m_Smokes = nullptr;
    int             m_SmokesCnt = 0;
    float           m_TTL = 0;

    CMatrixEffectShleif(void);
    virtual ~CMatrixEffectShleif();

public:
    friend class CMatrixEffect;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Tact(float);
    virtual void Release(void);
    virtual int  Priority(void) { return 30; };

    void AddSmoke(
        const D3DXVECTOR3& pos,
        float ttl,
        float puffttl,
        float spawntime,
        dword color,
        bool is_bright,
        float speed
    );
    void AddFire(
        const D3DXVECTOR3& pos,
        float ttl,
        float puffttl,
        float spawntime,
        float dispfactor,
        bool is_bright,
        float speed,
        SFloatRGBColor close_color = SFloatRGBColor{ 1.0f, 1.0f, 0.1f },
        SFloatRGBColor far_color = SFloatRGBColor{ 0.5f, 0.0f, 0.0f }
    );

    //void AddFlame(const D3DXVECTOR3& start, const D3DXVECTOR3& dir, const D3DXVECTOR3& speed, float ttl, dword hitmask, CMatrixMapStatic* skip, dword user, FIRE_END_HANDLER handler);

    void SetTTL(float ttl) { m_TTL = ttl; }
};

#endif