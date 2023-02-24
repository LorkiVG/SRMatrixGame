// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CBILLBOARD_HEADER
#define CBILLBOARD_HEADER

#include "D3DControl.hpp"

#define MAX_SPRITES           8192
#define MAX_SPRITE_TEX_GROUPS 64


#define BILLBOARD_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct SSpriteVertex
{
    D3DXVECTOR3 p = { 0.0f, 0.0f, 0.0f }; //Vertex position
    dword       color = 0;
    float       tu = 0.0f, tv = 0.0f; //Vertex texture coordinates
};

struct SSpriteTexture
{
    float tu0 = 0.0f, tv0 = 0.0f;
    float tu1 = 0.0f, tv1 = 0.0f;
};

class CSprite : public Base::CMain
{
    static CSprite* sprites[MAX_SPRITES];
    static int      sprites_left;
    static int      sprites_right;
    static CSprite* m_FirstIntense;
    static D3D_VB   m_VB;
    static D3D_IB   m_IB;

    static CTextureManaged* m_SortableTex;
    //static CTextureManaged* m_SingleTex;

    union {
        int m_Z = 0;
        CSprite* m_Next;// = nullptr  // list of sprites
    };
    CSprite* m_NextTex = nullptr;

    bool m_TexSingleFlag = false;

public:
#ifdef _DEBUG
    dword release_called = true;
    const char* m_file = nullptr;
    int         m_line = 0;
#endif
    D3DXVECTOR3 m_Pos = { 0.0f, 0.0f, 0.0f };    // sortable center

private:
    float m_Scale = 0.0f;  // sortable scale
    dword m_Color = 0xFFFFFFFF;  // diffuse color

    union
    {
        const SSpriteTexture* m_Tex = nullptr;
        CTextureManaged* m_TexSingle;// = nullptr;
    };

    D3DXMATRIX m_Rot;

    bool IsSingleBrightTexture(void) const { return m_TexSingleFlag; }
    void SetSingleBrightTexture(bool set) { m_TexSingleFlag = set; }
    
    void UpdateVBSlot(SSpriteVertex* vb, const D3DXMATRIX& iview);

public:
    CSprite() = default;
    CSprite(TRACE_PARAM_DEF const D3DXVECTOR3& pos, float scale, float angle, dword color, const SSpriteTexture* tex); //sorted
    CSprite(TRACE_PARAM_DEF const D3DXVECTOR3& pos, float scale, float angle, dword color, CTextureManaged* tex);      //single
    ~CSprite()
    {
        DTRACE();
        // do nothing!
#ifdef _DEBUG
        if (m_FirstIntense != nullptr)
            //if(m_FirstIntense != nullptr || m_Root != nullptr)
            //if(m_FirstIntense != nullptr || m_First != nullptr)
        {
            __asm int 3
        }
#endif
#ifdef _DEBUG
        if (!release_called)
        {
            __asm int 3
        }
#endif
    }

    static void StaticInit(void)
    {
        m_VB = nullptr;
        m_IB = nullptr;
        m_FirstIntense = nullptr;
        sprites_left = MAX_SPRITES >> 1;
        sprites_right = (MAX_SPRITES >> 1);

        m_SortableTex = nullptr;
        //m_SingleTex = nullptr;
    }

    static void Init(void); // prepare VB
    static void Deinit(void); // prepare VB

    void Release(void)
    {
    DTRACE();

#ifdef _DEBUG
        if(m_FirstIntense != nullptr) {  __asm int 3  }
        release_called = true;
#endif

    }

    static void SortEndDraw(const D3DXMATRIX& iview, const D3DXVECTOR3& campos);   // its actualy draw all sortable from sorted list

    void Sort(const D3DXMATRIX& sort);
    void SortIntense(void);

    void DrawNow(const D3DXMATRIX& iview);

    static void BeforeDraw(void) { m_SortableTex->Preload(); }

    void SetAngle(float angle, float dx, float dy)
    {
        D3DXMatrixRotationZ(&m_Rot, angle);
        //m_Rot._11 *= m_Scale; m_Rot._12 *= m_Scale; 
        //m_Rot._21 *= m_Scale; m_Rot._22 *= m_Scale; 
        m_Rot._41 = dx;
        m_Rot._42 = dy;
    }

    void SetPos(const D3DXVECTOR3& pos) { m_Pos = pos; }
    const D3DXVECTOR3& GetPos(void) { return m_Pos; }
    void DisplaceTo(const D3DXVECTOR2& d) { m_Rot._41 = d.x; m_Rot._42 = d.y; }
    void SetScale(float scale) { m_Scale = scale; }
    float GetScale(void) const { return m_Scale; }
    void SetAlpha(BYTE a) { m_Color = (m_Color & 0x00FFFFFF) | (a << 24); }
    void SetColor(dword c) { m_Color = c; }

    //static CTextureManaged* GetIntenseTex(void) { return m_SingleTex; }
    //static void SetTextures(CTextureManaged* st, CTextureManaged* it) { m_SortableTex = st; m_SingleTex = it; };
    static void SetSortTexture(CTextureManaged* st) { m_SortableTex = st; };
};

class CSpriteLine : public CMain
{
    static CSpriteLine* m_First;
    CSpriteLine* m_Next = nullptr;
    CSpriteLine* m_NextTex = nullptr;

    D3DXMATRIX m_Rot;

    float m_Width = 0.0f;
    dword m_Color = 0;
    CTextureManaged* m_Tex = nullptr;

    D3DXVECTOR3 m_Pos0 = { 0.0f, 0.0f, 0.0f }, m_Pos1 = { 0.0f, 0.0f, 0.0f };

#ifdef _DEBUG
public:
    bool release_called = true;
    const char* m_file = nullptr;
    int         m_line = 0;
#endif

public:
    CSpriteLine() = default; //ќбъ€вление нужно, т.к. в некоторых местах вызываетс€ именно дефолтный конструктор
    
    #ifdef _DEBUG
        CSpriteLine(TRACE_PARAM_DEF const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, float width, dword color, CTextureManaged* tex) : m_file(_file), m_line(_line), release_called(false) {}
    #endif
        CSpriteLine(TRACE_PARAM_DEF const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, float width, dword color, CTextureManaged* tex) : CMain(), m_Pos0(pos0), m_Pos1(pos1), m_Width(width), m_Color(color), m_Tex(tex) {}
    ~CSpriteLine() = default;

    static void StaticInit() { m_First = nullptr; }
    friend void CSprite::SortEndDraw(const D3DXMATRIX& iview, const D3DXVECTOR3& campos);

    void Release()
    {
    DTRACE();

#ifdef _DEBUG
        release_called = true;
#endif
        ASSERT(m_First == nullptr);
    }

    void DrawNow(const D3DXVECTOR3& campos);
    void AddToDrawQueue();
    void UpdateVBSlot(SSpriteVertex* vb, const D3DXVECTOR3& campos);

    void SetPos(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1) { m_Pos0 = pos0; m_Pos1 = pos1; }
    void SetWidth(float w) { m_Width = w; };
    void SetAlpha(byte a) { m_Color = (m_Color & 0x00FFFFFF) | (a << 24); }
    void SetColor(dword c) { m_Color = c; }

    const D3DXVECTOR3 & GetPos0() {return m_Pos0;}
    const D3DXVECTOR3 & GetPos1() {return m_Pos1;}
};

#endif