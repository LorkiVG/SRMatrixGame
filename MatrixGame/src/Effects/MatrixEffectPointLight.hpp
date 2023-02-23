// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


// point light
#define POINTLIGHT_FVF ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 )
#define POINTLIGHT_FVF_V ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

typedef struct
{
    D3DXVECTOR3 p;      // Vertex position
    dword       color;
    float       tu0, tv0; // Vertex texture coordinates
    float       tu1, tv1; // Vertex texture coordinates
    //float       tu;
} SPointLightVertex;

typedef struct
{
    D3DXVECTOR3 p;      // Vertex position
    dword       color;
} SPointLightVertexV;

#define POINTLIGHT_ALTITUDE (0.7f)

#define POINTLIGHT_BILLSCALE    1


struct SMatrixMapPoint;

struct SMapPointLight
{
    SMatrixMapPoint *mp;
    float            lum;
    int              addlum_r, addlum_g, addlum_b;
};

struct SPL_VBIB
{
private:
    static SPL_VBIB    *m_FirstFree;
    static SPL_VBIB    *m_LastFree;

    static SPL_VBIB    *m_FirstAll;
    static SPL_VBIB    *m_LastAll;

    D3D_VB      m_VB;
    D3D_IB      m_IB;

    int         m_VBSize;
    int         m_IBSize;


    SRemindCore  m_RemindCore;
    SPL_VBIB    *m_PrevFree;
    SPL_VBIB    *m_NextFree;
    SPL_VBIB    *m_PrevAll;
    SPL_VBIB    *m_NextAll;

    dword       m_FVF;

public:


    void NoNeed(void)
    {
        if (!IS_VB(m_VB)) 
        {
            Release();
        } else
        {
            LIST_ADD(this, m_FirstFree, m_LastFree, m_PrevFree, m_NextFree);
            m_RemindCore.Use(5000);
        }
    }
    void Release(void); // phisicaly delete

    static void StaticInit(void)
    {
        m_FirstFree = nullptr;
        m_LastFree = nullptr;
        m_FirstAll = nullptr;
        m_LastAll = nullptr;
    }
    static void ReleaseFree(void)
    {
        while (m_FirstFree) m_FirstFree->Release();
    }

    static void ReleaseBuffers(void)
    {
        SPL_VBIB * b = m_FirstAll;
        for (;b;b = b->m_NextAll)
        {
            b->DX_Free();
        }
    }

    static SPL_VBIB *GetCreate(int vbsize, int ibsize, dword fvf);


    void* LockVB(int vb_size);
    void* LockIB(int ib_size);

    void UnLockVB() {UNLOCK_VB(m_VB);};
    void UnLockIB() {UNLOCK_IB(m_IB);};

    D3D_VB  DX_GetVB(void) {return m_VB;};
    D3D_IB  DX_GetIB(void) {return m_IB;};

    void DX_Free(void)
    {
        if (IS_VB(m_VB)) DESTROY_VB(m_VB);
        if (IS_IB(m_IB)) DESTROY_IB(m_IB);
    }

    bool DX_Ready(void) const {return IS_VB(m_VB) && IS_IB(m_IB); }
};


class CMatrixEffectPointLight : public CMatrixEffect
{

    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };
    float       m_Radius = 0.0f;
    dword       m_Color = 0;
    dword       m_InitColor = 0;
    
    float       m_KillTime = 0.0f;
    float       m_Time = 0;
    float      _m_KT = 0.0f;

    int         m_NumVerts = 0;
    int         m_NumTris = 0;

    D3DXVECTOR2 m_RealDisp = { 0.0f, 0.0f };

    SPL_VBIB*   m_DX = nullptr;

    CTextureManaged* m_Tex = nullptr;
    
    CSprite*    m_Sprite = nullptr;

    CBuf        m_PointLum = nullptr;

    CMatrixEffectPointLight(const D3DXVECTOR3& pos, float radius, dword color, bool drawbill = false);
	virtual ~CMatrixEffectPointLight();

    void BuildLand(void);
    void BuildLandV(void);

    void UpdateData(void);
    void RemoveColorData(void);
    void AddColorData(void);

    void Clear(void);

    static void MarkAllBuffersNoNeed(void);

public:
    friend class CMatrixEffect;

    static void StaticInit(void)
    {
        SPL_VBIB::StaticInit();
    }
    static void ClearAll(void)
    {
        SPL_VBIB::ReleaseFree();
    }

    void SetPosAndRadius(const D3DXVECTOR3 &pos, float r)
    {
        ASSERT(r > 0);
        m_Pos = pos;
        m_Radius = r;
        UpdateData();
    }

    void SetPos(const D3DXVECTOR3 &pos)
    {
        m_Pos = pos;
        UpdateData();
    }

    void SetColor(dword c)
    {
        if(m_Sprite) m_Sprite->SetColor(c);
        RemoveColorData();
        m_Color = c;
        AddColorData();
    }

    void SetRadius(float r)
    {
        ASSERT(r > 0);
        m_Radius = r;
        UpdateData();
    }

    void Kill(float time);

    dword GetColor(void) { return m_Color; }

    virtual void BeforeDraw(void)
    {
        if(FLAG(m_before_draw_done, BDDF_PLIGHT)) return;
        if(m_Tex) m_Tex->Preload();

        //CTexture *tex = (CTexture *)g_Cache->Get(cc_Texture,TEXTURE_PATH_POINTLIGHT_INV, nullptr);
        //tex->Preload();
        
        SETFLAG(m_before_draw_done, BDDF_PLIGHT);
    };
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);

    virtual int  Priority(void);
};


