// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_EFFECT_REPAIR_INCLUDE
#define MATRIX_EFFECT_REPAIR_INCLUDE

#define REPAIR_BB_CNT   50

struct SRepairBBoard
{
    CSpriteLine bb;
    float       t = 0.0f;
    float       dt = 0.0f;
};

class CMatrixEffectRepair : public CMatrixEffect
{
    CTrajectory   m_Kord;
    CTrajectory   m_KordOnTarget;

    SRepairBBoard m_BBoards[REPAIR_BB_CNT];
    int           m_BBCnt = 0;

    CMatrixEffectRepair(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, float seekradius, CMatrixMapStatic* skip, ESpriteTextureSort sprite_spot);
	virtual ~CMatrixEffectRepair();

    SObjectCore* m_Target = nullptr;
    CMatrixMapStatic* m_Skip = nullptr;

    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Dir = { 0.0f, 0.0f, 0.0f };

    float m_OffTargetAmp = 0.0f;

    float m_ChangeTime = 0.0f;
    float m_SeekRadius = 0.0f;
    float m_NextSeekTime = 0.0f;
    float m_NextSpawnTime = 0.0f;
    float m_Time = 0.0;

    ESpriteTextureSort m_SpriteSpot = SPR_NONE;
    //ESpriteTextureSort m_SpriteRect = SPR_NONE;

public:
    friend class CMatrixEffect;

    void UpdateData(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir)
    {
        m_Pos = pos;
        m_Dir = dir;
        CMatrixEffectRepair::Tact(0);
    }

    CMatrixMapStatic* GetTarget();

    virtual void BeforeDraw();
    virtual void Draw();
    virtual void Tact(float step);
    virtual void Release();

    virtual int  Priority() { return MAX_EFFECT_PRIORITY; };
};

#endif