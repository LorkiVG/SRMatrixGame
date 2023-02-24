// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

extern IDirect3DDevice9* g_D3DD;

#define FREE_TEXTURE_TIME_PERIOD 7000
#define FREE_VO_TIME_PERIOD      30000

#include "Cache.hpp"
#include "D3DControl.hpp"
#include "BigVB.hpp"
#include "BigIB.hpp"
#include "ShadowProj.hpp"
#include "CBillboard.hpp"

#define GLOSS_TEXTURE_SUFFIX L"_gloss"

#define SHADOW_ALTITUDE (0.7f)

// resources
#define VOUF_MATRIX         SETBIT(0)
#define VOUF_SHADOWSTENCIL  SETBIT(1)
#define VOUF_RES            (VOUF_MATRIX | VOUF_SHADOWSTENCIL)

#define VOUF_MATRIX_USE     SETBIT(2)
#define VOUF_STENCIL_ON     SETBIT(3)

enum EObjectLoad
{
    OLF_NO_TEX,
    OLF_MULTIMATERIAL_ONLY,
    OLF_AUTO,
    OLF_SIMPLE
};

struct SVOSurface;
struct SSkin;

typedef const SSkin* (*SKIN_GET)(const wchar* tex, dword param);  // skin setup

typedef void (*SKIN_SETUP_TEX)(const SSkin* vo, dword user_param, int pass);
typedef bool (*SKIN_SETUP_STAGES)(const SSkin* vo, dword user_param, int pass);
typedef void (*SKIN_SETUP_CLEAR)(const SSkin* vo, dword user_param);
typedef void (*SKIN_SETUP_SHADOW)(const SSkin* vo);
typedef void (*SKIN_PRELOAD)(const SSkin* vo);

#define DEFAULT_ANIM_FRAME_PERIOD 100

// file formats

struct SVOHeader {
    dword m_Id;				// 0x00006f76
    dword m_Ver;			// Версия
    dword m_Flags;			// 1-16 битный индекс иначе 32 битный, 2-откуда брать текстуры
    dword dummy;
	int   m_MaterialCnt;	// Список материалов(текстур) SMaterial
    dword m_MaterialSme;	// Положение от начала заголовка
	int   m_GroupCnt;		// Инвормация по группам (Смещение верши и треугольников)
    dword m_GroupSme;
	int   m_VerCnt;			// Список всех вершин SVertexNorTex
    dword m_VerSme;
	int   m_TriCnt;			// Список всех треугольников. Кол-во индексов (3 индкса для каждого трегольника по 2 или 4 байта взависимости от флага)
    dword m_TriSme;
	int   m_FrameCnt;		// Список кадров SVOFrame
    dword m_FrameSme;
	int   m_AnimCnt;		// Список анимаций SVOAnimHeader
    dword m_AnimSme;
	int   m_MatrixCnt;		// Список матриц SVOExpMatrixHeader
    dword m_MatrixSme;
    dword m_EdgeCnt;		// Список всех граней
    dword m_EdgeSme;
};
struct SVOVertexNorTex {
	D3DXVECTOR3 v;
	D3DXVECTOR3 n;
	float tu,tv;
};

struct SVOMaterial {
    dword dummy;
	float dr,dg,db,da;
	float ar,ag,ab,aa;
	float sr,sg,sb,sa;
	float er,eg,eb,ea;
	float power;
	wchar tex_diffuse[32];
};

struct SVOGroup
{
    dword m_Material;
    dword m_Flags;			// 0-list
    dword m_VerCnt;
    dword m_VerStart;
    dword m_TriCnt;			// Кол-во индексов
    dword m_TriStart;
    dword dummy[2];
};

struct SVOFrame {
    dword m_GroupIndexCnt;
    dword m_GroupIndexSme;	// Каждый индекс 4-байтный указатель
	float m_CenterX, m_CenterY, m_CenterZ;
	float m_RadiusCenter;
	float m_MinX, m_MinY, m_MinZ;
	float m_MaxX, m_MaxY, m_MaxZ;
	float m_RadiusBox;
    dword m_EdgeCnt;
    dword m_EdgeStart;
    dword dummy;
};

struct SVOAnimHeader {
    dword m_Id;
	wchar m_Name[32];
    dword m_ModulesCount;
    dword m_ModuleSme;
    dword r1;
};

struct SVOMatrixHeader {
    dword m_Id;
	wchar m_Name[32];
    dword m_MatrixSme; // Спиок SVOMatrix   (кол-во по количеству m_FrameCnt)
    dword r1;
    dword r2;
};

struct SVOFrameEdge {
    dword m_SideTri1; // 0xf0000000 - triangle side		0x0ff00000 - group   0x000fffff - triangle
    dword m_SideTri2; // 0xf0000000 - triangle side		0x0ff00000 - group   0x000fffff - triangle
};

struct SVOEdge {
	int m_Tri1;
	int m_Tri2;
	byte m_Edge1;
	byte m_Enge2;
};

struct SVOEdgeGroup {
	int m_EdgeSme;
	int m_EdgeCnt;
};

////////////////
//memory VO

#define VO_FVF (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
struct SVOVertex
{
	D3DXVECTOR3 v = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 n = { 0.0f, 0.0f, 0.0f };
    float       tu = 0.0f;
    float       tv = 0.0f;

    static const dword FVF = VO_FVF;
};


struct SVOTriangle
{
    union
    {
        struct { int i0, i1, i2; };
        int i[3] = { 0 };
    };
    //D3DXVECTOR3 norm;
};

struct SVOIndices16
{
    word i0 = 0, i1 = 0, i2 = 0;
};

struct SVOIndices32
{
    dword i0 = 0, i1 = 0, i2 = 0;
};

struct SVOSurface
{
    const SSkin* skin = nullptr;
    CWStr texname = (CWStr)L"";

    //float dr, dg, db, da;
	//float ar, ag, ab, aa;
	//float sr, sg, sb, sa;
	//float er, eg, eb, ea;
	//float power;
};

struct SVOUnion
{
	int m_Surface = 0;
    int m_Base = 0;            //used for draw indexed primitives
    union
    {
	    int m_VerMinIndex = 0; // used for draw indexed primitives // always zero for optimized meshes (or negative, see m_IBase)
        int m_IBase;// = 0;    // negative value! 
    };
	int m_VerCnt = 0;          //used for draw indexed primitives
	int m_TriCnt = 0;
	int m_TriStart = 0;
};

struct SVOFrameModel
{
    D3DXVECTOR3 m_Min = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_Max = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_GeoCenter = { 0.0f, 0.0f, 0.0f };
    float       m_Radius = 0.0f;

    int         m_UnionStart = 0;
    int         m_UnionCnt = 0;
    int         m_EdgeStart = 0;
    int         m_EdgeCnt = 0;

    int         m_VerCnt = 0;    // used in pick
    int         m_VerStart = 0;  // used in pick
};

struct SVOFrameRuntime
{
    int m_EdgeVertexIndexMin = 0;
    int m_EdgeVertexIndexCount = 0;
};

struct SVOAnimation
{
    dword m_Id = 0;
    wchar m_Name[32] { L'\0' };
    dword m_FramesCnt = 0;
    dword m_FramesStart = 0;
};

struct SVOMatrix
{
    dword m_Id = 0;
	wchar m_Name[32] { L'\0' };
	dword m_MatrixStart = 0; // Спиок D3DXMATRIX  (кол-во по количеству кадров)
};

struct SVOFrameIndex
{
    int frame = 0;
    int time = 0;
};

struct SVONormal
{
    union
    {
        struct { byte x, y, z, s; };
        dword all = 0;
    };
};

struct SVOFrameEdgeModel
{
    int         v00 = 0, v01 = 0; //Verts of triangles with common edge (disp in bytes)
    SVONormal   n0, n1;   //Normals
};

struct SVOGeometrySimple
{
    struct SVOGS_asis
    {
        int         m_TriCnt = 0;
        int         m_VertsCnt = 0;
        D3DXVECTOR3 m_Mins = { 0.0f, 0.0f, 0.0f };
        D3DXVECTOR3 m_Maxs = { 0.0f, 0.0f, 0.0f };
        D3DXVECTOR3 m_Center = { 0.0f, 0.0f, 0.0f };
        float       m_Radius = 0.0f;

    } m_AsIs;

    SVOTriangle*    m_Tris = nullptr;
    D3DXVECTOR3*    m_Verts = nullptr;
};

struct SVOGeometry
{
    int* m_VerticesIdx = nullptr; // idxs for frames
    
    SBigVBSource<SVOVertex> m_Vertices;
    //SVOVertex*     m_Vertices;
    //int            m_VerticesCnt;

    SVOTriangle* m_Triangles = nullptr;
    int          m_TrianglesCnt = 0;
    //union
    //{
    //    SVOIndices16* m_Idx16;
    //    SVOIndices32* m_Idx32;
    //};
    //int            m_IdxCnt;
    SBigIBSource     m_Idxs;
    //int            m_IdxStride;    // 2 or 4

    dword*           m_UnionsIdx = nullptr;    // idxs for frames
    SVOUnion*        m_Unions = nullptr;
    int              m_UnionsCnt = 0;

    SVOSurface*      m_Surfaces = nullptr;
    int              m_SurfacesCnt = 0;

    SVOFrameEdgeModel* m_Edges = nullptr;
    int              m_EdgesCnt = 0;

    SVOFrameIndex*   m_FramesIdx = nullptr;    // idx for animations
    SVOFrameModel*   m_Frames = nullptr;
    int              m_FramesCnt = 0;

    SVOFrameRuntime* m_FramesRuntime = nullptr;

    SVOAnimation*    m_Animations = nullptr;
    int              m_AnimationsCnt = 0;

    SVOMatrix*       m_Matrixs = nullptr;
    int              m_MatrixsCnt = 0;

    D3DXMATRIX*      m_AllMatrixs = nullptr;
    //SVONormal*     m_Normals;
};

typedef bool (*ENUM_VERTS_HANDLER)(const SVOVertex& v, dword data);

class CVectorObjectAnim;
class CVectorObject : public CCacheData
{
    static CBigVB<SVOVertex>* m_VB;
    static CBigIB* m_IB;

    SVOGeometry        m_Geometry;
    SVOGeometrySimple* m_GeometrySimple = nullptr;

    CBlockPar       m_Props;

    static          SVOHeader* Header(const CBuf& buf) { return (SVOHeader*)buf.Get(); }

    bool PickSimple(const D3DXMATRIX& ma, const D3DXMATRIX& ima, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float* outt) const;

    SRemindCore     m_RemindCore;

public:
    friend class CVOShadowStencil;

    SVOGeometrySimple* GetGS(void) { return m_GeometrySimple; }

    static void StaticInit(void)
    {
        m_VB = nullptr;
        m_IB = nullptr;
    }

	CVectorObject(void);
	virtual ~CVectorObject();

    static      void DrawBegin(void) { m_IB->BeforeDraw(); m_VB->BeforeDraw(); ASSERT_DX(g_D3DD->SetFVF(VO_FVF)); }
    static      void DrawEnd(void) {}

    static void MarkAllBuffersNoNeed(void) { if(m_VB) m_VB->ReleaseBuffers(); if(m_IB) m_IB->ReleaseBuffers(); }

    //bool DX_IsEmpty(void) { return !IS_VB(m_IB); }
    void DX_Prepare(void) { DTRACE(); m_Geometry.m_Vertices.Prepare(m_VB); m_Geometry.m_Idxs.Prepare(m_IB); };
    void DX_Free(void) { m_Geometry.m_Vertices.MarkNoNeed(m_VB); m_Geometry.m_Idxs.MarkNoNeed(m_IB); }

    int GetFramesCnt(void) const                          { return m_Geometry.m_FramesCnt; }
    const SVOFrameModel* GetFrame(int no) const           { return m_Geometry.m_Frames + no; }

    const wchar* GetAnimName(int i)                       { return m_Geometry.m_Animations[i].m_Name; }

    int GetAnimCount(void)  const                         { return m_Geometry.m_AnimationsCnt; }
	int GetAnimById(dword id);
	int GetAnimByName(const wchar* name);
    int GetAnimFramesCount(int no) const    		      { return m_Geometry.m_Animations[no].m_FramesCnt; }
    int GetAnimFrameTime(int anim, int frame) const       { return abs(m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart + frame].time); }
    int GetAnimLooped(int anim) const                     { return m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart].time > 0; }
    int GetAnimFrameIndex(int anim, int frame) const      { return m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart + frame].frame; }

    CWStr GetPropValue(const wchar* name) const           { return m_Props.ParGetNE(name); }

    const D3DXVECTOR3& GetFrameGeoCenter(int frame) const { return m_Geometry.m_Frames[frame].m_GeoCenter; }

    int GetMatrixCount(void) const                        { return m_Geometry.m_MatrixsCnt; }
	const D3DXMATRIX* GetMatrixById(int frame, dword id) const;
	const D3DXMATRIX* GetMatrixByName(int frame, const wchar* name) const;
    const D3DXMATRIX* GetMatrix(int frame, int matrix) const { return m_Geometry.m_AllMatrixs + (m_Geometry.m_Matrixs + matrix)->m_MatrixStart + frame; }
	const wchar* GetMatrixNameById(dword id) const;
    const wchar* GetMatrixName(int idx) const             { return m_Geometry.m_Matrixs[idx].m_Name; }
    dword GetMatrixId(int idx) const                      { return m_Geometry.m_Matrixs[idx].m_Id; }

    bool Pick(int noframe, const D3DXMATRIX& ma, const D3DXMATRIX& ima, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float* outt) const;
    bool PickFull(int noframe, const D3DXMATRIX& ma, const D3DXMATRIX& ima, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float* outt) const;

    void EnumFrameVerts(int noframe, ENUM_VERTS_HANDLER evh, dword data) const;

    const CWStr& GetSurfaceFileName(int i) const { return m_Geometry.m_Surfaces[i].texname; }

	//void EdgeClear(void);
	//void EdgeBuild(void);
    //bool EdgeExist(void) { return m_Edge != nullptr; }

    void GetBound(int noframe, const D3DXMATRIX& objma, D3DXVECTOR3& bmin, D3DXVECTOR3& bmax) const;

    void CalcShadowProjMatrix(int noframe, SProjData& pd, D3DXVECTOR3& dir, float addsize);
    static void CalcShadowProjMatrix(int cnt, CVectorObjectAnim** obj, const  int* noframe, const D3DXMATRIX* wm, SProjData& pd, D3DXVECTOR3& dir, float addsize);
    static CTextureManaged* CalcShadowTexture(int cnt, CVectorObjectAnim** obj, const int* noframe, const D3DXMATRIX* wm, const SProjData& pd, int texsize, CVOShadowCliper* cliper, CBaseTexture* tex_to_update = nullptr);
    static CTextureManaged* CalcShadowTextureWOMat(int cnt, CVectorObjectAnim** obj, const int* noframe, const D3DXMATRIX* wm, int texsize, CVOShadowCliper* cliper, CBaseTexture* tex_to_update = nullptr);

    void BeforeDraw(void);
    void Draw(int noframe, dword user_param, const SSkin* ds);

    void LoadSpecial(EObjectLoad flags, SKIN_GET sg, dword gsp);
    void PrepareSpecial(EObjectLoad flags, SKIN_GET sg, dword gsp) { if(!IsLoaded()) LoadSpecial(flags, sg, gsp); }

    bool IsNoSkin(int surf) { return m_Geometry.m_Surfaces[surf].skin == nullptr; }

    virtual bool IsLoaded(void) { return (m_Geometry.m_Vertices.verts != nullptr); }
	virtual void Unload(void);    
    virtual void Load(void) { LoadSpecial(OLF_NO_TEX, nullptr, 0); }
};

struct SSkin
{
    SKIN_SETUP_TEX      m_SetupTex;
    SKIN_SETUP_STAGES   m_SetupStages;
    SKIN_SETUP_CLEAR    m_SetupClear;
    SKIN_SETUP_SHADOW   m_SetupTexShadow;   // used only for shadow projecting
    SKIN_PRELOAD        m_Preload;
};

struct SColorInterval
{
    dword c1, c2;
    int   time1, time2;
};

struct SLightData
{
    SColorInterval* intervals;
    int             intervals_cnt;
    int             time;
    int             period;
    dword           matid;

    byte            bbytes[sizeof(CSprite)];

    CSprite& BB(void) { return *(CSprite*)&bbytes; }
    
    void Release(void);  // just free internal data. not itself
};

class CVectorObjectAnim : public CMain
{
    CVectorObject* m_VO = nullptr;
    const SSkin* m_Skin = nullptr;

	int m_Time = 0;
	int m_TimeNext = 0;

	int m_Frame = 0;
    int m_VOFrame = 0;
    int m_Anim = 0;

    int m_AnimLooped = 1;

    SLightData* m_Lights = nullptr;
    int         m_LightsCount = 0;

    void SetLoopStatus(void)
    {
        if(m_VO->GetAnimLooped(m_Anim)) m_AnimLooped = 1;
        else m_AnimLooped = 0;
    }

public:
    CVectorObjectAnim(void) : CMain() {};
	~CVectorObjectAnim();

	void Clear(void);

    CVectorObject* VO(void)					{ return m_VO; }
    const SSkin*   GetSkin(void) const      { return m_Skin; }
    void           SetSkin(const SSkin* s)  { m_Skin = s; }

    void Init(CVectorObject* vo, CTextureManaged* tex_light, const SSkin* skin = nullptr);
    void InitLights(CTextureManaged* tex_light);

    bool IsAnim(const wchar* name) { return !(wcscmp(VO()->GetAnimName(m_Anim), name)); }

    CWStr GetAnimName() { return CWStr(VO()->GetAnimName(m_Anim)); }

    int  GetAnimIndex(void) const                           { return m_Anim; }
    void SetAnimDefault(void)                               { m_Anim = 0; FirstFrame(); SetLoopStatus();}
    void SetAnimDefault(int loop)                           { m_Anim = 0; FirstFrame(); m_AnimLooped = loop;}
    
    void SetAnimByIndex(int idx)                            { ASSERT(m_VO); m_Anim = idx; FirstFrame(); SetLoopStatus(); }
    void SetAnimByIndex(int idx, int loop)                  { ASSERT(m_VO); m_Anim = idx; FirstFrame(); m_AnimLooped = loop; }

    void SetAnimById(dword id)                              { ASSERT(m_VO); m_Anim =VO()->GetAnimById(id); if(m_Anim<0) m_Anim=0; FirstFrame(); SetLoopStatus();}
    void SetAnimById(dword id, int loop)                    { ASSERT(m_VO); m_Anim =VO()->GetAnimById(id); if(m_Anim<0) m_Anim=0; FirstFrame(); m_AnimLooped = loop;}
    
    bool SetAnimByName(const wchar* name)                   { ASSERT(m_VO); int i = VO()->GetAnimByName(name); if(i < 0) { return true; } m_Anim = i;  FirstFrame(); SetLoopStatus(); return false; }
    bool SetAnimByName(const wchar* name, int loop)         { ASSERT(m_VO); int i = VO()->GetAnimByName(name); if(i < 0) { return true; } m_Anim = i;  FirstFrame(); m_AnimLooped = loop; return false; }

    bool SetAnimByNameNoBegin(const wchar* name)            { ASSERT(m_VO); int i = VO()->GetAnimByName(name); if(i < 0) { return true; } SetLoopStatus(); if(m_Anim != i) { m_Anim = i;  FirstFrame(); } return false; }
    bool SetAnimByNameNoBegin(const wchar* name, int loop)  { ASSERT(m_VO); int i = VO()->GetAnimByName(name); if(i < 0) { return true; } m_AnimLooped = loop; if(m_Anim != i) { m_Anim = i;  FirstFrame(); } return false; }
    
    void SetAnimLooped(int loop)                            { m_AnimLooped = loop; }

    int GetVOFrame(void) const                              { return m_VOFrame; }
    int GetFrame(void) const                                { return m_Frame; }

	//int FrameCnt(void)							        { return m_AnimCnt; }
	//int Frame(void)								        { return m_Frame; }
	//void Frame(int zn)							        { m_Frame = zn; if(m_Frame < 0) m_Frame = 0; else if(m_Frame >= m_AnimCnt) m_Frame = 0; }
    const int GetMatrixCount(void)                              { return m_VO->GetMatrixCount(); }
    const D3DXMATRIX* GetMatrixById(dword id) const             { ASSERT(m_VO); return m_VO->GetMatrixById(m_VOFrame, id); }
    const D3DXMATRIX* GetMatrixByName(const wchar* name) const  { ASSERT(m_VO); return m_VO->GetMatrixByName(m_VOFrame, name); }
    const D3DXMATRIX* GetMatrix(int no) const                   { ASSERT(m_VO); return m_VO->GetMatrix(m_VOFrame, no); }

    void CalcShadowProjMatrix(int noframe, SProjData& pd, D3DXVECTOR3& dir, float addsize)
    {
        m_VO->CalcShadowProjMatrix(noframe, pd, dir, addsize);
    }

    void EnumFrameVerts(ENUM_VERTS_HANDLER evh, dword data) const { m_VO->EnumFrameVerts(m_VOFrame, evh, data); };

	bool Tact(int cms, float speed_factor = 0.0f); //Возвращает true, если анимация объекта сменила фрейм
    dword NextFrame(void);
    dword PrevFrame(void);
    void FirstFrame(void) { m_Frame = 0; m_VOFrame = VO()->GetAnimFrameIndex(m_Anim, 0); m_TimeNext = m_Time + VO()->GetAnimFrameTime(m_Anim, 0); }
    bool IsAnimEnd(void) const { return !m_AnimLooped && m_Frame == (m_VO->GetAnimFramesCount(m_Anim) - 1); }

    void GetBound(D3DXVECTOR3& bmin, D3DXVECTOR3& bmax) const;

    bool Pick(const D3DXMATRIX& ma, const D3DXMATRIX& ima, const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const;
    bool PickFull(const D3DXMATRIX& ma, const D3DXMATRIX& ima, const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const;

    void BeforeDraw(void);
    void Draw(dword user_param) { VO()->Draw(m_VOFrame, user_param, m_Skin); }
    void DrawLights(bool now, const D3DXMATRIX& objma, const D3DXMATRIX* iview);
};


class CVectorObjectGroup;

class CVectorObjectGroupUnit : public CMain {
public:
	dword m_Flags = VOUF_MATRIX | VOUF_SHADOWSTENCIL; // Какой ресурс объекта изменился. При созданнии класса устанавливается в 0xffffffff

	CVectorObjectGroup* m_Parent = nullptr;
	CVectorObjectGroupUnit* m_Prev = nullptr;
	CVectorObjectGroupUnit* m_Next = nullptr;

	int m_Id = 0;

    CWStr m_Name = (CWStr)L"";

    CVectorObjectAnim* m_Obj = HNew(g_CacheHeap) CVectorObjectAnim();

	CVectorObjectGroupUnit* m_Link = nullptr;
	int m_LinkMatrixId = -1; // -1-center -2-by name >=0-by id
	CWStr m_LinkMatrixName = (CWStr)L"";

	D3DXMATRIX m_Matrix;				// Дополнительная матрица позиционирования в локальных координатах.

	D3DXMATRIX m_MatrixWorld;			// Конечная матрица трансформации объекта в мировые координаты.
    D3DXMATRIX m_IMatrixWorld;

	CVOShadowStencil* m_ShadowStencil = nullptr;

public:
    CVectorObjectGroupUnit()
    {
        D3DXMatrixIdentity(&m_Matrix);
        D3DXMatrixIdentity(&m_MatrixWorld);
    }
    ~CVectorObjectGroupUnit();
        
	void GetResources(dword need);	//Запрашиваем необходимые ресурсы объекта
    void RChange(dword zn) { m_Flags |= VOUF_RES & zn; } //Меняем указанные ресурсы объекта

	void ShadowStencilOn(bool zn = true);
};

class CVectorObjectGroup : public CMain {
public:
	CVectorObjectGroupUnit* m_First = nullptr;
	CVectorObjectGroupUnit* m_Last = nullptr;

	D3DXMATRIX* m_GroupToWorldMatrix = nullptr;	//Матрица позиционирования группы в мировом пространстве.

	CWStr m_Name = (CWStr)L"";				    //Имя файла

    D3DXVECTOR3 m_ShadowStencilLight = { 0.0f, 0.0f, -1.0f };
	//D3DXPLANE m_ShadowStencilCutPlane = { 0.0f, 0.0f, 1.0f, 0.0f };
    float       m_GroundZ = 0.0f; // for stencil shadow len

public:
    CVectorObjectGroup() = default;
    ~CVectorObjectGroup() { Clear(); }

	void Clear() { while(m_First) Delete(m_Last); }

    void Delete(CVectorObjectGroupUnit* un);
    CVectorObjectGroupUnit* Add(void);
    CVectorObjectGroupUnit* GetByName(const wchar* name);
    CVectorObjectGroupUnit* GetByNameNE(const wchar* name);
    CVectorObjectGroupUnit* GetById(int id);
    CVectorObjectGroupUnit* GetByIdNE(int id);

    D3DXVECTOR3 GetPosByName(const wchar* name) const;

    //void ChangeSetupFunction(OBJECT_SETUP_TEX setup_tex, OBJECT_SETUP_STAGES setup_stages);
	void GetResources(dword need);		// Запрашиваем нужные ресурсы объекта
	void RChange(dword zn);				// Указываем какие ресурсы изменились
	void RChangeByLink(CVectorObjectGroupUnit* link, dword zn);

    bool IsAlreadyLoaded(void) { return m_First != nullptr; }

	bool Tact(int cms);

	CVectorObjectGroupUnit* Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const;

    void EnumFrameVerts_transform_position(ENUM_VERTS_HANDLER evh, dword data) const;

	void BoundGet(D3DXVECTOR3& bmin, D3DXVECTOR3& bmax);
	void BoundGetAllFrame(D3DXVECTOR3& bmin, D3DXVECTOR3& bmax);

	void ShadowStencilOn(bool zn = true);
        
    void ShadowStencil_DX_Free(void);

	void Load(const wchar* filename, CTextureManaged* lt, SKIN_GET sg, dword gsp);

    void BeforeDraw(bool proceed_shadows);
	void Draw(dword user_param);
    void DrawLights(bool now = false, const D3DXMATRIX* iview = nullptr);
	void ShadowStencilDraw(void);
};