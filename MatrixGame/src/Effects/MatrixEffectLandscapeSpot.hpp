// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// landscape spot
class CMatrixEffectLandscapeSpot;


#define LANDSCAPESPOT_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
struct SLandscapeSpotVertex
{
    D3DXVECTOR3 p = { 0.0f, 0.0f, 0.0f }; // Vertex position
    dword       color = 0;                // Vertex color

    // Vertex texture coordinates
    float       tu = 0.0f;
    float       tv = 0.0f;
};


#define SPOT_ALTITUDE (0.7f)
#define SPOT_SIZE     (5.0f)



class CMatrixEffectLandscapeSpot : public CMatrixEffect
{
    static CMatrixEffectLandscapeSpot* m_First;
    static CMatrixEffectLandscapeSpot* m_Last;

    CMatrixEffectLandscapeSpot* m_Next = nullptr;
    CMatrixEffectLandscapeSpot* m_Prev = nullptr;

    D3D_VB m_VB = nullptr;
    D3D_IB m_IB = nullptr;

    float  m_DX = 0.0f;
    float  m_DY = 0.0f;

    SSpotProperties* m_Props = nullptr;
    CTextureManaged* m_Texture = nullptr;

    float            m_LifeTime = 0.0f;
    
    int              m_CntVerts = 0;
    int              m_CntTris = 0;

    byte*            m_IndsPre = nullptr;
    byte*            m_VertsPre = nullptr;

    dword            m_Color = 0;

    void BuildLand(const D3DXVECTOR2& pos, float angle, float scalex, float scaley, float addz, bool scale_by_normal);

    CMatrixEffectLandscapeSpot(const D3DXVECTOR2& pos, float angle, float scale, ESpotType type);
	virtual ~CMatrixEffectLandscapeSpot();
    void DrawActual();

#ifdef _DEBUG
    const char* _file;
    int         _line;
#endif


    bool PrepareDX();
    static void StaticInit()
    {
        m_First = nullptr;
        m_Last = nullptr;
    }

    static void MarkAllBuffersNoNeed();

public:
    friend class CMatrixEffect;

    static void SpotTactConstant(CMatrixEffectLandscapeSpot* spot, float tact);
    static void SpotTactAlways(CMatrixEffectLandscapeSpot* spot, float tact);
    static void SpotTactPlasmaHit(CMatrixEffectLandscapeSpot* spot, float tact);
    static void SpotTactMoveTo(CMatrixEffectLandscapeSpot* spot, float tact);
    static void SpotTactPointlight(CMatrixEffectLandscapeSpot* spot, float tact);
    static void SpotTactVoronka(CMatrixEffectLandscapeSpot* spot, float tact);

    virtual void BeforeDraw() { PrepareDX(); m_Texture->Preload(); };
    virtual void Draw() {};
    virtual void Tact(float step);
    virtual void Release();

#ifdef _DEBUG
    void BeforeRelease(const char* file, int line) {_file = file; _line = line;};
#endif

    virtual int  Priority();

    static void DrawAll();

    void SetColor(dword color) {m_Color = color;}
    dword GetColor() const { return m_Color;}


#ifdef DEAD_CLASS_SPY_ENABLE
    int DCS_UnderSpy() const { return sizeof(CMatrixEffectLandscapeSpot); }
#endif
};

