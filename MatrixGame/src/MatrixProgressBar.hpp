// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef PROGRESS_BAR_INCLUDE
#define PROGRESS_BAR_INCLUDE

#define PB_BUILDING_WIDTH   200
#define PB_ROBOT_WIDTH      70
#define PB_CANNON_WIDTH     100
#define PB_FLYER_WIDTH      100
#define PB_SPECIAL_WIDTH    50
//#define PB_BUILDING_HEIGHT  8


#define PB_COLOR_0  0x00FF0000
#define PB_COLOR_1  0x00FFFF00
#define PB_COLOR_2  0x0000FF00

#define PB_Z    0

struct SPBPos
{
    dword   present;
    float   m_X;
    float   m_Y;
    float   m_Width;
};

enum EPBCoord
{
    PBC_ORIGINAL = 0,
    PBC_CLONE1,
    PBC_CLONE2,

};

class CMatrixProgressBar : public CMain
{
    static CMatrixProgressBar* m_First;
    static CMatrixProgressBar* m_Last;
    static CMatrixProgressBar* m_FirstClones;
    static CMatrixProgressBar* m_LastClones;
    static CTextureManaged* m_Tex;

    CMatrixProgressBar* m_Prev = nullptr;
    CMatrixProgressBar* m_Next = nullptr;

    SPBPos* m_Coord = nullptr;
    int     m_CoordCount = 1;

    float   m_Pos = 0.0f;

    void DrawClones(bool pbd);
    void Draw(bool pbd);
    void Draw(SPBPos* pos, bool pbd);

public:
    CMatrixProgressBar()
    {
        if(m_Tex == nullptr)
        {
            m_Tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_PB);
            m_Tex->MipmapOff();
        }
        m_Tex->RefInc();

        m_Coord = (SPBPos*)HAlloc(sizeof(SPBPos), g_MatrixHeap);
        m_CoordCount = 1;
        m_Coord[0].present = true;

        //Modify(x,y,width, height, pos);
        m_Coord->m_X = float(g_ScreenX + 1);

        LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
    }
    ~CMatrixProgressBar()
    {
        m_Tex->RefDecUnload();
        if(m_Tex->Ref() <= 0) m_Tex = nullptr;

        HFree(m_Coord, g_MatrixHeap);

        if(m_CoordCount > 1)
        {
            LIST_DEL(this, m_FirstClones, m_LastClones, m_Prev, m_Next);
        }
        else
        {
            LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
        }
    }

    void Modify(float x, float y, float width, /* float height, */ float pos) { Modify(x, y); m_Coord->m_Width = (float)floor(width); /* m_Height = floor(height); */ m_Pos = pos; };
    void Modify(float x, float y, float pos) { Modify(x, y); m_Pos = pos; };
    void Modify(float x, float y) { m_Coord->m_X = (float)floor(x) + 0.5f; m_Coord->m_Y = (float)floor(y) + 0.5f; };
    void Modify(float pos) { m_Pos = pos; };

    void CreateClone(EPBCoord pbc, float x, float y, float width);
    void KillClone(EPBCoord pbc);
    bool ClonePresent(EPBCoord pbc) const { return (m_CoordCount>pbc) && (m_Coord[pbc].present != 0); };

    static void BeforeDrawAll(void) { m_Tex->Preload(); }

    static void DrawAll(void);
    static void DrawAllClones(void);

    static void StaticInit(void)
    {
        m_First = nullptr;
        m_Last = nullptr;
        m_FirstClones = nullptr;
        m_LastClones = nullptr;
        m_Tex = nullptr;
    }
};

#endif