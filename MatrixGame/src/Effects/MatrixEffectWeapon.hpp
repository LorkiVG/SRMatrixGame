// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// weapon
#define FLAME_PUFF_TTL 2000

#define LASER_WIDTH 10
#define VOLCANO_FIRE_LENGHT 15
#define VOLCANO_FIRE_WIDTH 10
#define VOLCANO_FIRE_KONUS_RADIUS 5
#define VOLCANO_FIRE_KONUS_LENGTH 5

extern float g_WeaponDamageNormalCoef;
extern float g_WeaponDamageArcadeCoef;
extern float g_UnitSpeedArcadeCoef;

//#define WD_PLASMA           300
//#define WD_VOLCANO          300
//#define WD_HOMING_MISSILE   500
//#define WD_BOMB             400
//#define WD_FLAMETHROWER     100
//#define WD_BIGBOOM          100
//#define WD_LIGHTENING       300
//#define WD_LASER            200
//#define WD_GUN              300

#define WEAPON_MAX_HEAT      1000

enum EWeapon
{
    WEAPON_NONE = 0,

    WEAPON_MACHINEGUN = 1,
    WEAPON_FLAMETHROWER = 4,
    WEAPON_MORTAR = 5,
    WEAPON_LASER = 6,
    WEAPON_BOMB = 7,
    WEAPON_PLASMAGUN = 8,
    WEAPON_DISCHARGER = 9,
    WEAPON_REPAIRER = 10,
    WEAPON_TURRET_LASER = 13,

    WEAPON_INSTANT_DEATH = -1
};

class CLaser : public CMatrixEffect
{
    CSpriteLine m_Sprites;
    CSprite     m_end;

    virtual ~CLaser()
    {
        m_Sprites.Release();
        m_end.Release();
    }

public:
    CLaser(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1);

    virtual void BeforeDraw(void) {};
    virtual void Draw(void);
    virtual void Tact(float) {};
    virtual void Release(void)
    {
        HDelete(CLaser, this, m_Heap);
    };
    virtual int  Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1)
    {
        m_Sprites.SetPos(pos0, pos1);
        m_end.m_Pos = pos0;
    }
};

class CVolcano : public CMatrixEffect
{
    CMatrixEffectKonus m_Konus;
    CSpriteLine        m_bl1;
    CSpriteLine        m_bl2;
    CSpriteLine        m_bl3;

    virtual ~CVolcano()
    {
        m_bl1.Release();
        m_bl2.Release();
        m_bl3.Release();
    }

public:
    CVolcano(const D3DXVECTOR3& start, const D3DXVECTOR3& dir, float angle);

    virtual void BeforeDraw(void)
    {
        m_Konus.BeforeDraw();
    };
    virtual void Draw(void);
    virtual void Tact(float) {};
    virtual void Release(void)
    {
        HDelete(CVolcano, this, m_Heap);
    };

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, const D3DXVECTOR3& dir)
    {
        m_bl1.SetPos(pos0, pos1);
        m_bl2.SetPos(pos0, pos1);
        m_bl3.SetPos(pos0, pos1);
        m_Konus.Modify(pos0, dir);
    }
    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, const D3DXVECTOR3& dir, float angle)
    {
        m_bl1.SetPos(pos0, pos1);
        m_bl2.SetPos(pos0, pos1);
        m_bl3.SetPos(pos0, pos1);
        m_Konus.Modify(pos0, dir, VOLCANO_FIRE_KONUS_RADIUS, VOLCANO_FIRE_KONUS_LENGTH, angle);
    }
};

class CMatrixEffectWeapon : public CMatrixEffect
{
    int    m_WeaponNum = 0;
    float  m_WeaponDist = 0.0f;
    float  m_WeaponCoefficient = 1.0f;

    dword  m_Sound = SOUND_ID_EMPTY;
    ESound m_ShotSoundType = S_NONE;

    dword  m_User = 0;
    FIRE_END_HANDLER m_Handler = nullptr;
    int    m_Ref = 1;

    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Dir = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Speed = { 0.0f, 0.0f, 0.0f };

    float  m_Time = 0.0f;
    float  m_CoolDown = 0.0f;
    CMatrixMapStatic* m_Skip = nullptr;

    int    m_FireCount = 0;

    SEffectHandler m_Effect;

    union
    {
        CVolcano*            m_Volcano = nullptr;
        CLaser*              m_Laser;// = nullptr;
        CMatrixEffectRepair* m_Repair;// = nullptr;
    };

    SObjectCore* m_Owner = nullptr;
    int          m_SideStorage = 0; // side storage (if owner will be killed)

    CMatrixEffectWeapon(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, dword user, FIRE_END_HANDLER handler, int type, int cooldown);
    virtual ~CMatrixEffectWeapon();

    void FireWeapon(void);

public:
    D3DXVECTOR3 GetPos()            { return m_Pos; }
    void  SetDefaultCoefficient()   { m_WeaponCoefficient = g_WeaponDamageNormalCoef; }
    void  SetArcadeCoefficient()    { m_WeaponCoefficient = g_WeaponDamageArcadeCoef; }
    int   GetWeaponNum(void) const  { return m_WeaponNum; }
    float GetWeaponDist(void) const { return m_WeaponDist * m_WeaponCoefficient; }
    friend class CMatrixEffect;

    static void WeaponHit(CMatrixMapStatic* hiti, const D3DXVECTOR3& pos, dword user, dword flags);
    static void SelfRepairEffect();

    void SetOwner(CMatrixMapStatic* ms);
    int  GetSideStorage(void) const { return m_SideStorage; };
    CMatrixMapStatic* GetOwner(void);

    virtual void BeforeDraw(void) {};
    virtual void Draw(void) {};
    virtual void Tact(float step);
    virtual void Release(void);

    int GetShotsDelay() { return m_CoolDown; };

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void Modify(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, const D3DXVECTOR3& speed);

    bool IsFire(void) { return FLAG(m_Flags, WEAPFLAGS_FIRE); }
    bool IsFireWas(void) { bool r = FLAG(m_Flags, WEAPFLAGS_FIREWAS); RESETFLAG(m_Flags, WEAPFLAGS_FIREWAS); return r; }
    bool IsHitWas(void) { bool r = FLAG(m_Flags, WEAPFLAGS_HITWAS); RESETFLAG(m_Flags, WEAPFLAGS_HITWAS); return r; }

    void ResetFireCount(void) { m_FireCount = 0; }
    int GetFireCount(void) const { return m_FireCount; }

    void ModifyCoolDown(float addk) { m_CoolDown += m_CoolDown * addk; }
    void ModifyDist(float addk) { m_WeaponDist += m_WeaponDist * addk; }

    void FireBegin(const D3DXVECTOR3& speed, CMatrixMapStatic* skip)
    {
        if(IsFire()) return;

        m_Speed = speed;
        SETFLAG(m_Flags, WEAPFLAGS_FIRE);
        RESETFLAG(m_Flags, WEAPFLAGS_FIREWAS);
        RESETFLAG(m_Flags, WEAPFLAGS_HITWAS);

        m_Skip = skip;

        //CHelper::Create(10000, 0)->Line(m_Pos, m_Pos + m_Dir * 100);
    }
    void FireEnd(void);

    static void SoundHit(int weapon_num, const D3DXVECTOR3& pos);
};