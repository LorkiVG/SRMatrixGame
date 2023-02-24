// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_INSTANT_DRAW_INCLUDE
#define MATRIX_INSTANT_DRAW_INCLUDE

struct SVertBase
{
};

struct SVert_V3_C_UV : public SVertBase
{
    D3DXVECTOR3 p = { 0.0f, 0.0f, 0.0f };
    dword       color = 0;
    float       tu = 0.0f;
    float       tv = 0.0f;
};

struct SVert_V4_UV : public SVertBase
{
    D3DXVECTOR4 p = { 0.0f, 0.0f, 0.0f, 0.0f };
    float       tu = 0.0f;
    float       tv = 0.0f;
};

struct SVert_V4_C : public SVertBase
{
    D3DXVECTOR4 p = { 0.0f, 0.0f, 0.0f, 0.0f };
    dword       col = 0;
};

struct SVert_V4 : public SVertBase
{
    D3DXVECTOR4 p = { 0.0f, 0.0f, 0.0f, 0.0f };
};

enum E_FVF
{
    IDFVF_V3_C_UV,
    IDFVF_V4_UV,
    IDFVF_V4_C,
    IDFVF_V4,

    IDFVF_CNT
};

struct  SOneSet
{
    void*         accum = nullptr;
    int           accumcntalloc = 0;
    int           accumcnt = 0;
    CBaseTexture* tex = nullptr;
    dword         tf = 0;
    dword         tf_used = 0;
};

struct SFVF_VB
{
    dword    fvf = 0;
    int      stride = 0;
    int      statistic = 0;
    int      statistic_max_tex = 0;
    int      cursize = 0; // size of currently allocated VB
    D3D_VB   vb;
    SOneSet* sets = nullptr;
    int      sets_cnt = 0;
    int      sets_alloc = 0;
    int      disp = 0;
};

class CInstDraw : public CMain
{
    static SFVF_VB m_FVFs[IDFVF_CNT];
    static E_FVF   m_Current;
    static D3D_IB  m_IB;
    static int     m_IB_Count;


public:
    static void StaticInit(void)
    {
        memset(m_FVFs, 0, sizeof(m_FVFs));
        m_FVFs[IDFVF_V3_C_UV].fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
        m_FVFs[IDFVF_V3_C_UV].stride = sizeof(SVert_V3_C_UV);

        m_FVFs[IDFVF_V4_UV].fvf = D3DFVF_XYZRHW | D3DFVF_TEX1;
        m_FVFs[IDFVF_V4_UV].stride = sizeof(SVert_V4_UV);

        m_FVFs[IDFVF_V4_C].fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
        m_FVFs[IDFVF_V4_C].stride = sizeof(SVert_V4_C);

        m_FVFs[IDFVF_V4].fvf = D3DFVF_XYZRHW;
        m_FVFs[IDFVF_V4].stride = sizeof(SVert_V4);

        m_IB = nullptr;
        m_IB_Count = 0;

#ifdef _DEBUG
        m_Current = IDFVF_CNT;
#endif
    }


    static void DrawFrameBegin();
    static void BeginDraw(E_FVF fvf);
    static void AddVerts(void* v, CBaseTexture* tex); // add 4 verts
    static void AddVerts(void* v, CBaseTexture* tex, dword tf); // add 4 verts
    static void ActualDraw();

    static void MarkAllBuffersNoNeed();

    static  void ClearAll();
};


#endif