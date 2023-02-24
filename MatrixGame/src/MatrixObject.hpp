// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "ShadowStencil.hpp"
#include "MatrixShadowManager.hpp"

enum EMapObjectBehaviorOnDamage
{
    BEHF_STATIC,       // не реагирует на окружающую действительность
    BEHF_BURN,         // горит (и сгорает)
    BEHF_BREAK,        // ломается
    BEHF_ANIM,         // меняется анимация
    BEHF_SENS,         // чувствительность к приближению роботов
    BEHF_SPAWNER,      // робот - spawner
    BEHF_TERRON,       // босс
    BEHF_PORTRET       //Видимо, для пасхалки в тайной комнате (в коде не использовано)
};

struct SMatrixSkin;

class CMatrixMapObject : public CMatrixMapStatic {

    struct SObjectShadowTexture
    {
        CTextureManaged* tex = nullptr;
        CTextureManaged* tex_burn = nullptr;
    };

public:
	float m_AngleZ = 0.0f;
    float m_AngleX = 0.0f;
    float m_AngleY = 0.0f;
	float m_Scale = 1.0f;
    float m_TexBias = -1.0f;

    int m_Type = -1;

    int m_UID = -1;

    D3DXVECTOR3 m_ShCampos = { 0, 0, 0};
    D3DXVECTOR2 m_ShDim = { 0, 0 };

    EShadowType m_ShadowType = SHADOW_PROJ_STATIC;//SHADOW_STENCIL;

    static SObjectShadowTexture* m_ShadowTextures;
    static int                   m_ShadowTexturesCount;

    EMapObjectBehaviorOnDamage m_BehaviorFlag = BEHF_STATIC;
    union
    {
        struct
        {
            int          m_BreakHitPoint;
            int          m_AnimState; // for BEHF_ANIM
            CMatrixProgressBar* m_ProgressBar;
            int          m_BreakHitPointMax;
            int          m_NextExplosionTime;
            int          m_NextExplosionTimeSound;
        };

        struct
        {
            int          m_NextTime;
            int          m_BurnTimeTotal;    // increases

            SMatrixSkin* m_BurnSkin;
            int          m_BurnSkinVis;  // 0-255
        };

        struct
        {
            // -1 - not inited
            // 0 - default (idle)
            int         m_PrevStateRobotsInRadius;

            SObjectCore* m_SpawnRobotCore;
            float        m_SensRadius;
            int          m_Photo;
            int          m_PhotoTime;
        };
    };

    CVectorObjectAnim* m_Graph = nullptr;
    CVOShadowStencil* m_ShadowStencil = nullptr;
    CMatrixShadowProj* m_ShadowProj = nullptr;

    void FreeShadowTexture(void);

public:
    CMatrixMapObject() : CMatrixMapStatic(), m_BreakHitPoint(0), m_AnimState(0), m_ProgressBar(nullptr), m_BreakHitPointMax(0), m_NextExplosionTime(0), m_NextExplosionTimeSound(0)
    {
        m_Core->m_Type = OBJECT_TYPE_MAPOBJECT;
    }
    ~CMatrixMapObject()
    {
        if(m_Graph) UnloadObject(m_Graph, Base::g_MatrixHeap);
        if(m_ShadowStencil) HDelete(CVOShadowStencil, m_ShadowStencil, Base::g_MatrixHeap);
        if(m_ShadowProj)
        {
            FreeShadowTexture();
            HDelete(CMatrixShadowProj, m_ShadowProj, Base::g_MatrixHeap);
        }
        if(m_BehaviorFlag == BEHF_TERRON && m_ProgressBar != nullptr) HDelete(CMatrixProgressBar, m_ProgressBar, Base::g_MatrixHeap);
        if(m_BehaviorFlag == BEHF_BREAK && m_ProgressBar != nullptr) HDelete(CMatrixProgressBar, m_ProgressBar, Base::g_MatrixHeap);
        if(m_BehaviorFlag == BEHF_ANIM && m_ProgressBar != nullptr) HDelete(CMatrixProgressBar, m_ProgressBar, Base::g_MatrixHeap);
        if(m_BehaviorFlag == BEHF_SPAWNER && m_SpawnRobotCore) m_SpawnRobotCore->Release();
    }

    static void StaticInit(void)
    {
        m_ShadowTextures = nullptr;
        m_ShadowTexturesCount = 0;
    }
#ifdef _DEBUG
    static void ValidateAfterReset(void)
    {
        if(m_ShadowTextures || m_ShadowTexturesCount) _asm int 3
    }
#endif

    static void InitTextures(int n);
    static void ClearTextures(void);
    static void ClearTexture(CBaseTexture* tex);

    void SetupMatricesForShadowTextureCalc(void);
    void MarkSpecialShadow(void) { SETFLAG(m_ObjectFlags, OBJECT_STATE_SHADOW_SPECIAL); }
    void Init(int ids);
    //void InitAsBaseRuins(CMatrixBuilding* b, const CWStr& namev, const CWStr& namet, bool shadow);
    void InitAsBaseRuins(const D3DXVECTOR2& pos, int angle, const CWStr& namev, const CWStr& namet, bool shadow);

	virtual void GetResources(dword need);

	virtual void Tact(int cms);
    virtual void LogicTact(int );
    void PauseTact(int cms);

    void ApplyAnimState(int anims);

    virtual bool Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt)  const;
    bool PickFull(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt)  const;

    virtual bool TakingDamage(int weap, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, int attacker_side = NEUTRAL_SIDE, CMatrixMapStatic* attaker = nullptr);

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void DrawShadowStencil(void);
    virtual void DrawShadowProj(void);

    virtual void FreeDynamicResources(void);

    void OnLoad(void);

    virtual bool CalcBounds(D3DXVECTOR3& omin, D3DXVECTOR3& omax);

    virtual bool FitToMask(dword mask)
    {
        if(mask & TRACE_OBJECT) return true;
        return false;
    }

    virtual int GetSide(void) const { return 0; };
    virtual bool NeedRepair(void) const { return false; }

    virtual bool InRect(const CRect& rect) const;

    void OnOutScreen(void) {};
};

