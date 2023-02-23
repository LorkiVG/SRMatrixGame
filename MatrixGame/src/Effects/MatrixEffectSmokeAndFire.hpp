// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixEffect.hpp"

// smoke

#define MAX_PUFF_COUNT      256
#define PUFF_SCALE1  4.0f
#define PUFF_SCALE2  16.0f

#define PUFF_FIRE_SCALE  3.5f


#define  FIRE_STREAM_WIDTH  5

#define  FIREFRAME_ANIM_PERIOD  100
#define  FIREFRAME_TTL_POROG    0.9f
#define  FIREFRAME_W_DEAD       0
#define  FIREFRAME_H_DEAD       0

//___________________________________________________________________________________________________
/////////////////////////////////////////////////////////////////////////////////////////////////////

struct SSmokePuff
{
    CSprite     m_Puff;
    float       m_PuffAngle = 0.0f;
    float       m_PuffTTL = 0.0f;
    D3DXVECTOR3 m_PuffOrig = { 0.0f, 0.0f, 0.0f };
};

struct SFirePuff : public SSmokePuff
{
    D3DXVECTOR2 m_PuffDir = { 0.0f, 0.0f };
};

class CMatrixEffectSmoke : public CMatrixEffect
{
    SSmokePuff* m_Puffs = nullptr;
    int         m_PuffCnt = 0;

    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Mins = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Maxs = { 0.0f, 0.0f, 0.0f };

    float m_TTL = 0.0;
    float m_PuffTTL = 0.0;
    float m_Spawn = 0.0;

    float m_Time = 0.0;
    float m_NextSpawnTime = 0.0;

    float m_Speed = 0.0;
    dword m_Color = 0;

    CMatrixEffectSmoke(
        const D3DXVECTOR3& pos,
        float ttl,
        float puffttl,
        float spawntime,
        dword color,
        bool is_bright = false,
        float speed = SMOKE_SPEED
    );
	virtual ~CMatrixEffectSmoke();

    void SpawnPuff(void);

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectFire;
    friend class CMatrixEffectExplosion;
    friend class CMatrixEffectShleif;

    void SetPos(const D3DXVECTOR3 &pos) { m_Pos = pos; }
    void SetTTL(float ttl) { m_TTL = ttl; }

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);
    virtual int  Priority(void) { return 30; };
};

class CMatrixEffectFire : public CMatrixEffect
{
    SFirePuff*  m_Puffs = nullptr;
    int         m_PuffCnt = 0;

    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Mins = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Maxs = { 0.0f, 0.0f, 0.0f };

    float m_TTL = 0.0f;
    float m_PuffTTL = 0.0f;
    float m_Spawn = 0.0f;

    float m_Time = 0.0f;
    float m_NextSpawnTime = 0.0f;

    float m_Speed = 0.0f;
    float m_DispFactor = 0.0f;

    SFloatRGBColor m_CloseColor = { 0.0f, 0.0f, 0.0f };
    SFloatRGBColor m_FarColor = { 0.0f, 0.0f, 0.0f };

    CMatrixEffectFire(
        const D3DXVECTOR3& pos,
        float ttl,
        float puffttl,
        float spawntime,
        float dispfactor,
        bool is_bright = false,
        float speed = SMOKE_SPEED,
        const SFloatRGBColor& close_color = { 1.0f, 1.0f, 0.1f },
        const SFloatRGBColor& far_color = { 0.5f, 0.0f, 0.0f }
    );

    void SpawnPuff(void);

public:
    ~CMatrixEffectFire();

    friend class CMatrixEffect;
    friend class CMatrixEffectSmoke;
    friend class CMatrixEffectExplosion;
    friend class CMatrixEffectShleif;

    void SetPos(const D3DXVECTOR3& pos) { m_Pos = pos; }
    void SetTTL(float ttl) { m_TTL = ttl; }

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);
    virtual int  Priority(void) { return 30; };
};

class CMatrixEffectFireStream : public CMatrixEffect
{
    byte m_SpritesCount;
    CSpriteLine* m_Sprites;

    virtual ~CMatrixEffectFireStream()
    {
        for(int i = 0; i < m_SpritesCount; ++i) m_Sprites[i].Release();
        delete[] m_Sprites;
        m_Sprites = nullptr;
    }

public:
    CMatrixEffectFireStream(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, const std::vector<int>& sprites_num = { SPR_JET_STREAM_ANIM_FRAME_1 , SPR_JET_STREAM_ANIM_FRAME_2, SPR_JET_STREAM_ANIM_FRAME_3, SPR_JET_STREAM_ANIM_FRAME_4, SPR_JET_STREAM_ANIM_FRAME_5 });

    virtual void BeforeDraw(void);
    virtual void Draw(void) { Draw(false); };
    void Draw(bool now);
    virtual void Tact(float) {};
    virtual void Release(void)
    {
        HDelete(CMatrixEffectFireStream, this, m_Heap);
    };

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1) { for(int i = 0; i < m_SpritesCount; ++i) m_Sprites[i].SetPos(pos0, pos1); }
};


class CMatrixEffectFireAnim : public CMatrixEffect
{
    byte m_SpritesCount;
    CSpriteLine* m_Sprites;

    float m_TTLcurr;
    float m_TTL;
    int m_Frame;
    int m_NextTime;
    float m_Height;
    float m_Width;
    float m_HeightCurr;
    float m_WidthCurr;
    D3DXVECTOR3 m_Pos;

    virtual ~CMatrixEffectFireAnim()
    {
        ELIST_DEL(EFFECT_FIREANIM);

        for(int i = 0; i < m_SpritesCount; ++i) m_Sprites[i].Release();
        delete[] m_Sprites;
        m_Sprites = nullptr;
    }

    void Update(void)
    {
        for(int i = 0; i < m_SpritesCount; ++i)
        {
            m_Sprites[i].SetPos(m_Pos, m_Pos + D3DXVECTOR3(0, 0, m_HeightCurr));
            m_Sprites[i].SetWidth(m_WidthCurr);
        }
    }

public:
    CMatrixEffectFireAnim(const D3DXVECTOR3& pos, float anim_width, float anim_height, int time_to_live,
        const std::vector<int>& sprites_id = { SPR_FLAME_ANIM_FRAME_1, SPR_FLAME_ANIM_FRAME_2, SPR_FLAME_ANIM_FRAME_3, SPR_FLAME_ANIM_FRAME_4, SPR_FLAME_ANIM_FRAME_5, SPR_FLAME_ANIM_FRAME_6, SPR_FLAME_ANIM_FRAME_7, SPR_FLAME_ANIM_FRAME_8 }, int sprites_count = 8);

    virtual void BeforeDraw(void);
    virtual void Draw(void) { Draw(false); };
    void Draw(bool now);
    virtual void Tact(float);
    virtual void Release(void) { HDelete(CMatrixEffectFireAnim, this, m_Heap); };

    virtual int Priority(void) { return 10; };

    void SetPos(const D3DXVECTOR3& pos)
    {
        m_Pos = pos;
        Update();
    }

    void SetWH(float w, float h)
    {
        m_WidthCurr = w; m_Width = w;
        m_HeightCurr = h; m_Height = h;
        Update();
    }
};